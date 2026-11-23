// split_screen.cpp — Zombies local split screen (top / bottom), opt-in with BO2_SPLIT=1. Zombies target only.
// Without BO2_SPLIT every hook below tail-calls the original function: solo play is unchanged.
//
// Design: SCR_UpdateFrame keeps its single local client. The client draw (0x55af10) is run twice per frame:
//   pass A in the top half (player 1), the render is sent, then pass B in the bottom half with player 2's camera.
// Player 2 is a server test client (bot), added on the server thread once in game, only when a second pad is present
// (a second game controller, or BO2_PAD2_SCRIPT). Pad 2 drives the bot's usercmd (SV_BotUserMove hook).
// The half is set by writing the view rectangle into cgs before each pass (view()); the 2D placement is recomputed by
// the game for each half (place()). The renderer side (final copy per half, depth clear before B, 2D boxing) lives in
// remaining_dlls_impl.cpp (ss_viewport) and metal_backend.mm (prof_efface).
// Logs: [SPLIT], [SPLIT-P2].
#include "remill/Arch/X86/Runtime/State.h"
#include "bo2_getenv.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <unordered_map>

extern "C" uint8_t* bo2_g_base;
extern "C" uint32_t win32_shim_alloc(uint32_t size, uint32_t align);
extern "C" uint32_t bo2_pad2_state(uint8_t out[12]);
extern "C" bool bo2_pad2_present();
extern "C" void bo2_pad2_lobby_seen();   // runtime/input/pad_gc.mm: lobby shown, player 2 may join (Start or A)
extern "C" void bo2_pad2_lock();         // ... match start: no more joining or leaving

// Hooked functions (lifted copies, renamed _impl).
extern "C" void* sub_55af10_impl(State*, uint32_t, void*);   // client draw (eax = local client)
extern "C" void* sub_41c800_impl(State*, uint32_t, void*);   // send the frame's render (flags)
extern "C" void* sub_41bb80_impl(State*, uint32_t, void*);   // render scene (refdef)
extern "C" void* sub_41bb10_impl(State*, uint32_t, void*);   // frame view parms (refdef)
extern "C" void* sub_6ec9f0_impl(State*, uint32_t, void*);   // server frame: bot commands
extern "C" void* sub_5d6b30_impl(State*, uint32_t, void*);   // SV_BotUserMove(client, cmd)
extern "C" void* sub_4ad550_impl(State*, uint32_t, void*);   // FPS counter draw (once per frame)
extern "C" void* sub_9345b0_impl(State*, uint32_t, void*);   // LUI update
extern "C" void* sub_9315c0_impl(State*, uint32_t, void*);   // LUI draw
extern "C" void* sub_696fe0_impl(State*, uint32_t, void*);   // lobby member list
extern "C" void* sub_522bf0_impl(State*, uint32_t, void*);   // third person (local client, force)
extern "C" void* sub_457970_impl(State*, uint32_t, void*);   // model lighting cache lookup
extern "C" void* sub_4106f0_impl(State*, uint32_t, void*);   // add model to scene
extern "C" void* sub_4dd1b0_impl(State*, uint32_t, void*);   // interaction prompt draw
extern "C" void* sub_539990_impl(State*, uint32_t, void*);   // first person view model
extern "C" void* sub_4adaa0_impl(State*, uint32_t, void*);   // revive icon over downed players
extern "C" void* sub_4ca660_impl(State*, uint32_t, void*);   // script hud elements
// Game functions called directly.
extern "C" void* sub_41eb80(State*, uint32_t, void*);   // end of frame command list
extern "C" void* sub_6e6780(State*, uint32_t, void*);   // SV_AddTestClient() -> gentity or 0
extern "C" void* sub_57e7d0(State*, uint32_t, void*);   // 2D placement (place, x, y, w, h, aspect, flag)
extern "C" void* sub_966510(State*, uint32_t, void*);   // 2D placement flag for the local client
extern "C" void* sub_5dc400(State*, uint32_t, void*);   // ClientEndFrame(gentity)
extern "C" void* sub_457da0(State*, uint32_t, void*);   // upload queued model lighting (entries, n)
extern "C" void* sub_553910(State*, uint32_t, void*);   // R_RegisterFont(path, 1)
extern "C" void* sub_4274b0(State*, uint32_t, void*);   // font name -> path
extern "C" void* sub_6f7370(State*, uint32_t, void*);   // localize a key, 0 if unknown
extern "C" void* sub_4dd100(State*, uint32_t, void*);   // interaction prompt text (esi = local client)
extern "C" void* sub_545e90(State*, uint32_t, void*);   // stretched pic (13 args)
extern "C" void* sub_5530f0(State*, uint32_t, void*);   // UI_DrawText (12 args)
extern "C" void* sub_8a9790(State*, uint32_t, void*);   // BG_UpdatePlayerDObj
extern "C" void* sub_787b80(State*, uint32_t, void*);   // allocate an anim tree
extern "C" void* sub_53c600(State*, uint32_t, void*);   // view model DObj update (lc, 0)
extern "C" void* sub_4efc50(State*, uint32_t, void*);   // rebuild the scoreboard rows (lc)

namespace {
constexpr uint32_t kClientState = 0x10e7e18;   // 10 = in game
constexpr uint32_t kCgsPtr = 0x105f738, kCgPtr = 0x105f75c, kCgEntsPtr = 0x1060e48, kClientsPtr = 0x10e8344;
constexpr uint32_t kCgToCgs = 0x87700, kCgToEnts = 0xa5040;
constexpr uint32_t kRefdef = 0x4d890, kRefdefW = 0x4d898;
constexpr uint32_t kSendReturn = 0x55c46c;   // a return address inside SCR_UpdateFrame, used for nested calls
constexpr uint32_t kMaxClients = 0x2504468;  // com_maxclients dvar, value at +0x18
constexpr uint32_t kEntities = 0x210f640;    // g_entities, 0x31c each; gclient at +0x154
constexpr uint32_t kLevelMaxClients = 0x2286a48;
constexpr uint32_t kPlace = 0x11e2950;       // 2D placement of local client 0, 0x7c bytes
constexpr uint32_t kBackEnd = 0x33487d8, kParity = 0x33487e4, kLightToggle = 0x3509dd0;
constexpr uint32_t kSkinCache = 0x3942d50, kSkinCacheOn = 0x3942d80;
constexpr uint32_t kWhite = 0x105f768, kVariantDefs = 0x316d0f8;
constexpr uint32_t kFontBig = 0xb10950, kFontSmall = 0xb19c28;   // "extrabigfont", "smallFont"
constexpr uint32_t kSession = 0x11c7550;
constexpr uint32_t kVmArr = 0xfc83e8, kVmWiArr = 0xfc83f8, kDobjHandles = 0x254d0d0;
constexpr uint32_t kAnimCallback = 0x6e7720, kVmReturn = 0x52593f;
constexpr uint32_t kRdOrg = 0x34, kRdAxis = 0x44, kRdEnd = 0x68;

inline uint32_t g32(uint32_t a) { uint32_t v; memcpy(&v, bo2_g_base + a, 4); return v; }
inline void w32(uint32_t a, uint32_t v) { memcpy(bo2_g_base + a, &v, 4); }
inline bool pow2(uint64_t n) { return n && !(n & (n - 1)); }
uint32_t f2u(float f) { uint32_t u; memcpy(&u, &f, 4); return u; }

// Set on the first frame in game when player 2 did not join in the lobby: every hook passes through and the solo game
// plays full screen, as without BO2_SPLIT.
bool g_solo = false;
bool on() {
    static const bool v = [] {
        const bool e = bo2_getenv("BO2_SPLIT") != nullptr;
        if (e) fprintf(stderr, "[SPLIT] armed: a second pad joins player 2 with Start or A in the lobby\n");
        return e;
    }();
    return v && !g_solo;
}

bool g_ready = false, g_refused = false, g_sent = false, g_pass_b = false, g_in_cg = false;
uint32_t g_w = 0, g_h = 0;
float g_aspect = 0;
uint64_t g_passes = 0;
float g_aim_a[6];                        // player 1's eye and forward axis, read in pass A
uint32_t g_bot = 0;                      // player 2's gentity (0 = none)
volatile bool g_bot_requested = false;   // set by the draw at pass 64, served on the server thread
volatile uint32_t g_bot_cl = 0;          // player 2's server client: last cmd at +0x514c
uint8_t g_rd_before[kRdEnd - kRdOrg];
uint32_t g_rd_set = 0;                   // refdef holding player 2's camera, 0 otherwise
float g_cam_ang[2];                      // player 2's camera pitch, yaw
uint32_t g_frame_ms_a = 0;               // frame time of pass A (pass B sees 0)

uint32_t bot_gclient() { return g_bot ? g32(g_bot + 0x154) : 0; }
bool p2_live() { return on() && bot_gclient(); }

// Call a cdecl guest function on a scratch frame below the live stack. Restores esp; eax holds the result.
template <size_t N>
void* call(State* s, void* mem, void* (*fn)(State*, uint32_t, void*), uint32_t va, uint32_t below,
           const uint32_t (&args)[N], uint32_t ret = kSendReturn) {
    const uint32_t sp = s->gpr.rsp.dword, frame = (sp - below) & ~0xfu;
    w32(frame, ret);
    for (size_t i = 0; i < N; ++i) w32(frame + 4 + 4 * i, args[i]);
    s->gpr.rsp.dword = frame;
    mem = fn(s, va, mem);
    s->gpr.rsp.dword = sp;
    return mem;
}
void* call0(State* s, void* mem, void* (*fn)(State*, uint32_t, void*), uint32_t va, uint32_t below,
            uint32_t ret = kSendReturn) {
    const uint32_t sp = s->gpr.rsp.dword, frame = (sp - below) & ~0xfu;
    w32(frame, ret);
    s->gpr.rsp.dword = frame;
    mem = fn(s, va, mem);
    s->gpr.rsp.dword = sp;
    return mem;
}

// First in-game draw: full screen size, layout check.
bool prepare() {
    const uint32_t cgs = g32(kCgsPtr), cg = g32(kCgPtr), ents = g32(kCgEntsPtr);
    g_w = g32(cgs + 8); g_h = g32(cgs + 0xc);
    memcpy(&g_aspect, bo2_g_base + cgs + 0x10, 4);
    fprintf(stderr, "[SPLIT] cg=0x%08x cgs=0x%08x ents=0x%08x clients=0x%08x view %ux%u aspect %.3f\n", cg, cgs, ents,
            g32(kClientsPtr), g_w, g_h, g_aspect);
    if (!g_w || !g_h || cgs != cg + kCgToCgs || ents != cg + kCgToEnts) {
        fprintf(stderr, "[SPLIT] unexpected layout, split screen off\n");
        return false;
    }
    return true;
}

// View rectangle in cgs: pixels {x, y, w, h} at +0..+0xc, aspect at +0x10, screen fractions at +0x14..+0x20.
// half: 0 = top, 1 = bottom, -1 = full screen.
void view(int half) {
    const uint32_t cgs = g32(kCgsPtr);
    float f[4] = {0.f, half > 0 ? 0.5f : 0.f, 1.f, 0.5f};
    if (half < 0) { f[1] = 0.f; f[3] = 1.f; }
    const float a = g_aspect * f[2] / f[3];
    w32(cgs + 0, (uint32_t)(f[0] * g_w)); w32(cgs + 4, (uint32_t)(f[1] * g_h));
    w32(cgs + 8, (uint32_t)(f[2] * g_w)); w32(cgs + 0xc, (uint32_t)(f[3] * g_h));
    memcpy(bo2_g_base + cgs + 0x10, &a, 4);
    memcpy(bo2_g_base + cgs + 0x14, f, sizeof f);
}

// 2D placement for a half, computed by the game on the half's rectangle at 0,0, then the real-position fields
// (viewport +0x38, visible min/max +0x48/+0x50, adjustable min/max +0x58/+0x60) shifted. -1 restores the original.
uint8_t g_place[0x7c];
bool g_place_set = false;
void* place(State* s, void* mem, int half) {
    if (half < 0) {
        if (g_place_set) memcpy(bo2_g_base + kPlace, g_place, sizeof g_place);
        g_place_set = false;
        return mem;
    }
    if (!g_place_set) { memcpy(g_place, bo2_g_base + kPlace, sizeof g_place); g_place_set = true; }
    const uint32_t eax = s->gpr.rax.dword;   // the local client for the draw: keep it
    mem = call(s, mem, sub_966510, 0x966510, 0x140, {0u});
    const uint32_t flag = s->gpr.rax.dword & 0xff, w = g_w, h = g_h / 2, y = half ? h : 0;
    mem = call(s, mem, sub_57e7d0, 0x57e7d0, 0x140, {kPlace, 0u, 0u, w, h, f2u(g_aspect), flag});
    s->gpr.rax.dword = eax;
    for (uint32_t o : {0x38u, 0x48u, 0x50u, 0x58u, 0x60u}) {
        float v[2];
        memcpy(v, bo2_g_base + kPlace + o, sizeof v);
        v[1] += (float)y;
        memcpy(bo2_g_base + kPlace + o, v, sizeof v);
    }
    return mem;
}

// Player 2's camera: eye = bot origin (+0x28) + eye height (+0x208); aim = delta_angles (+0x98) + last cmd angles.
bool put_camera(uint32_t rd) {
    const uint32_t gc = bot_gclient();
    if (g_rd_set || !g_pass_b || !gc || rd != g32(kCgPtr) + kRefdef) return false;
    float ax[9];
    memcpy(ax, bo2_g_base + rd + kRdAxis, sizeof ax);
    const float n0 = ax[0] * ax[0] + ax[1] * ax[1] + ax[2] * ax[2], n2 = ax[6] * ax[6] + ax[7] * ax[7] + ax[8] * ax[8];
    if (!(n0 > 0.98f && n0 < 1.02f && n2 > 0.98f && n2 < 1.02f)) return false;   // not the expected layout
    memcpy(g_rd_before, bo2_g_base + rd + kRdOrg, sizeof g_rd_before);
    g_rd_set = rd;
    float o[3], ang[2], eye_h;
    memcpy(o, bo2_g_base + gc + 0x28, sizeof o);
    memcpy(ang, bo2_g_base + gc + 0x98, sizeof ang);
    if (const uint32_t cl = g_bot_cl) {
        ang[0] += (int32_t)g32(cl + 0x514c + 0xc) * (360.0f / 65536);
        ang[1] += (int32_t)g32(cl + 0x514c + 0x10) * (360.0f / 65536);
    }
    g_cam_ang[0] = ang[0]; g_cam_ang[1] = ang[1];
    memcpy(&eye_h, bo2_g_base + gc + 0x208, 4);
    const float p = ang[0] * 0.017453293f, yw = ang[1] * 0.017453293f;
    const float cp = cosf(p), sp = sinf(p), cy = cosf(yw), sy = sinf(yw);
    const float eye[3] = {o[0], o[1], o[2] + eye_h};
    const float axes[9] = {cp * cy, cp * sy, -sp, -sy, cy, 0, sp * cy, sp * sy, cp};   // AngleVectors, no roll
    memcpy(bo2_g_base + rd + kRdOrg, eye, sizeof eye);
    memcpy(bo2_g_base + rd + kRdAxis, axes, sizeof axes);
    return true;
}
void restore_camera() {
    if (!g_rd_set) return;
    memcpy(bo2_g_base + g_rd_set + kRdOrg, g_rd_before, sizeof g_rd_before);
    g_rd_set = 0;
}

// Upload the model lighting updates queued by pass B (only the first send of a frame uploads them).
void* upload_b_lighting(State* s, void* mem) {
    const uint32_t bd = g32(kBackEnd), n = bd ? g32(bd + 0x462c68) : 0;
    if (!n) return mem;
    return call(s, mem, sub_457da0, 0x457da0, 0x200, {bd + 0xdec68, n});
}
}  // namespace

// ---- server side -------------------------------------------------------------------------------------------------

// Server frame: the bot is added here, on the server thread, like the addtestclient script builtin.
extern "C" __attribute__((used)) void* bo2_ss_sv(State* s, uint32_t pc, void* mem) {
    if (g_bot_requested && !g_bot) {
        g_bot_requested = false;
        mem = call0(s, mem, sub_6e6780, 0x6e6780, 0x120, g32(s->gpr.rsp.dword));
        g_bot = s->gpr.rax.dword;
        fprintf(stderr, "[SPLIT-P2] player 2 added: %08x\n", g_bot);
    }
    return sub_6ec9f0_impl(s, pc, mem);
}

namespace {
// The game only runs ClientEndFrame for the first level.maxclients clients (1, set at map load): run it for the bot.
void* end_frame_bot(State* s, void* mem) {
    if (!g_bot || !bo2_g_base[g_bot + 0xfb] || g32(kLevelMaxClients) > 1 || !g32(g_bot + 0x154)) return mem;
    return call(s, mem, sub_5dc400, 0x5dc400, 0x200, {g_bot});
}

// Keep the bot's pm_type sane after being revived, and lying down while down.
void fix_down_state(uint32_t cmd) {
    const uint32_t gc = bot_gclient(), f = g32(gc + 0xc), t = g32(gc + 4);
    static uint32_t t_down = 0;
    static bool revived = false;
    if (f & 0x80000) { t_down = t; revived = false; }
    else if (t_down) { revived = t_down == 8; t_down = 0; }
    if (revived && (t == 7 || t == 8)) w32(gc + 4, 0);
    if (revived && t == 0) revived = false;
    if (f & 0x80000) w32(cmd + 4, g32(cmd + 4) | 0x800000u);
}

// Pad 2 -> cmd: sticks to move (+0x24 forward, +0x25 right) and aim (+0xc pitch, +0x10 yaw), buttons in +4.
void apply_pad2(uint32_t cmd) {
    struct { uint16_t buttons; uint8_t lt, rt; int16_t lx, ly, rx, ry; } p;
    static_assert(sizeof p == 12, "XINPUT_GAMEPAD");
    if (!bo2_pad2_state(reinterpret_cast<uint8_t*>(&p))) return;
    static auto before = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    const double dt = std::min(0.1, std::chrono::duration<double>(now - before).count());
    before = now;
    auto axis = [](int v, int dead) { return std::abs(v) < dead ? 0.0 : v / 32767.0; };
    const double fwd = axis(p.ly, 7849), right = axis(p.lx, 7849), yaw = axis(p.rx, 8689), pitch = axis(p.ry, 8689);
    bo2_g_base[cmd + 0x24] = (uint8_t)(int8_t)std::lround(fwd * 127);
    bo2_g_base[cmd + 0x25] = (uint8_t)(int8_t)std::lround(right * 127);
    w32(cmd + 0x10, g32(cmd + 0x10) - (uint32_t)(int32_t)std::lround(yaw * 180.0 * dt * 65536 / 360));
    w32(cmd + 0xc, g32(cmd + 0xc) - (uint32_t)(int32_t)std::lround(pitch * 120.0 * dt * 65536 / 360));
    // Game pad layout: RT fire, LT aim, A jump, B crouch, X use/reload, Y next weapon, LB tactical, RB grenade,
    // L3 sprint, R3 melee.
    uint32_t b = 0;
    if (p.rt > 30) b |= 0x80000000u;
    if (p.lt > 30) b |= 0x100000u;
    if (p.buttons & 0x1000) b |= 0x200000u;
    if (p.buttons & 0x2000) b |= 0x400000u;
    if (p.buttons & 0x4000) b |= 0x4000000u;
    if (p.buttons & 0x0100) b |= 0x10000u;
    if (p.buttons & 0x0200) b |= 0x20000u;
    if (p.buttons & 0x0040) b |= 0x40000000u;
    if (p.buttons & 0x0080) b |= 0x20000000u;
    w32(cmd + 4, g32(cmd + 4) | b);
    const uint32_t gc = bot_gclient();
    if (g32(gc + 0xc) & 0x80000) bo2_g_base[cmd + 0x24] = bo2_g_base[cmd + 0x25] = 0;   // down: no walking
    // Held weapons: gclient +0x264, 0x1c each, 15 slots; lethal grenade at +0x1a4 is skipped.
    static uint16_t y_before = 0;
    if ((p.buttons & 0x8000) && !y_before) {
        const uint32_t cur = g32(cmd + 0x18);
        int k = 0;
        for (int i = 0; i < 15; ++i) if (g32(gc + 0x264 + 0x1c * i) == cur) k = i;
        for (int j = 1; j < 15; ++j) {
            const uint32_t w = g32(gc + 0x264 + 0x1c * ((k + j) % 15));
            if (w && w != cur && w != g32(gc + 0x1a4)) { w32(cmd + 0x18, w); break; }
        }
    }
    y_before = p.buttons & 0x8000;
    // The bot repeats its last cmd: a newly held weapon goes into the cmd; one no longer held falls back to the first.
    static uint32_t held_before[15] = {};
    uint32_t held[15], pick = 0, first = 0;
    bool still_held = false;
    for (int i = 0; i < 15; ++i) {
        held[i] = g32(gc + 0x264 + 0x1c * i);
        if (!held[i] || held[i] == g32(gc + 0x1a4)) continue;
        if (!first) first = held[i];
        if (held[i] == g32(cmd + 0x18)) still_held = true;
        if (!pick && std::find(held_before, held_before + 15, held[i]) == held_before + 15) pick = held[i];
    }
    if (held_before[0] || held_before[1]) {
        const uint32_t w = pick ? pick : still_held ? 0 : first;
        if (w && w != g32(cmd + 0x18)) w32(cmd + 0x18, w);
    }
    memcpy(held_before, held, sizeof held);
}
}  // namespace

extern "C" __attribute__((used)) void* bo2_ss_botcmd(State* s, uint32_t pc, void* mem) {
    if (!on()) return sub_5d6b30_impl(s, pc, mem);
    const uint32_t sp = s->gpr.rsp.dword, cl = g32(sp + 4), cmd = g32(sp + 8);
    g_bot_cl = cl;
    mem = end_frame_bot(s, mem);
    mem = sub_5d6b30_impl(s, pc, mem);
    if (!bot_gclient()) return mem;
    fix_down_state(cmd);
    apply_pad2(cmd);
    return mem;
}

// ---- render send -------------------------------------------------------------------------------------------------

extern "C" __attribute__((used)) void* bo2_ss_send(State* s, uint32_t pc, void* mem) {
    if (!on()) return sub_41c800_impl(s, pc, mem);
    // A second client slot, opened only in game once the bot exists (2 in the lobby blocks loading).
    if (g_bot && g32(kClientState) == 10) {
        const uint32_t d = g32(kMaxClients);
        if (d && g32(d + 0x18) == 1) w32(d + 0x18, 2);
    }
    if (g_ready) {   // like the original two-client loop: a send after a send of the same frame is not the first
        const uint32_t sp = s->gpr.rsp.dword;
        uint32_t f = g32(sp + 4);
        if (g_sent) f &= ~1u;
        w32(sp + 4, f);
        g_sent = !(f & 2u);
        if (!(f & 1u)) mem = upload_b_lighting(s, mem);
    }
    return sub_41c800_impl(s, pc, mem);
}

// ---- lobby: player 2 shown in the first free member slot while the list is built ----------------------------------

extern "C" __attribute__((used)) void* bo2_ss_lobby(State* s, uint32_t pc, void* mem) {
    constexpr uint32_t kMembers = kSession + 0x208, kSize = 0x148;
    if (on() && g32(s->gpr.rsp.dword + 4) == kSession) bo2_pad2_lobby_seen();
    if (!on() || g32(s->gpr.rsp.dword + 4) != kSession || !bo2_pad2_present()) return sub_696fe0_impl(s, pc, mem);
    int p1 = -1, free_slot = -1;
    for (int i = 0; i < 18 && free_slot < 0; ++i) {
        const bool taken = bo2_g_base[kMembers + kSize * i] >= 3;
        if (taken && p1 < 0) p1 = i;
        else if (!taken && p1 >= 0) free_slot = i;
    }
    if (free_slot < 0) return sub_696fe0_impl(s, pc, mem);
    uint8_t* m = bo2_g_base + kMembers + kSize * free_slot;
    uint8_t saved[kSize];
    memcpy(saved, m, kSize);
    memcpy(m, bo2_g_base + kMembers + kSize * p1, kSize);
    snprintf(reinterpret_cast<char*>(m + 0x40), 0x20, "Player 2");
    uint64_t x;
    memcpy(&x, m + 0x38, 8); ++x; memcpy(m + 0x38, &x, 8);
    mem = sub_696fe0_impl(s, pc, mem);
    memcpy(m, saved, kSize);
    return mem;
}

// ---- 2D: HUD of both players, drawn after the FPS counter (once per frame) -----------------------------------------

namespace {
bool readable(uint32_t a) {
    if (a < 0x10000 || a > 0xc0000000u) return false;
    for (int i = 0; i < 3; ++i) if (bo2_g_base[a + i] < 0x20 || bo2_g_base[a + i] > 0x7e) return false;
    return true;
}

struct Hud {
    State* s;
    void* mem;
    uint32_t frame, color, text, big, small;

    void draw_text(uint32_t font, const char* t, float x, float y, uint32_t ah, uint32_t av, float h, const float (&c)[4],
                   uint32_t style = 0) {   // style 3 = shadowed
        memcpy(bo2_g_base + color, c, sizeof c);
        snprintf(reinterpret_cast<char*>(bo2_g_base + text), 0x80, "%s", t);
        const float e = h / (float)std::max<int32_t>(1, (int32_t)g32(font + 4));
        mem = call(s, mem, sub_5530f0, 0x5530f0, 0,
                   {kPlace, text, 0x7fffffffu, font, f2u(x), f2u(y), ah, av, f2u(e), f2u(e), color, style});
    }
    void weapon_name(uint32_t w, char* n, size_t size) {
        *n = 0;
        const uint32_t vd = w && w < 2048 ? g32(kVariantDefs + 4 * w) : 0;
        if (!readable(vd ? g32(vd) : 0)) return;
        uint32_t shown = g32(vd + 0xc);
        if (readable(shown)) {
            if (bo2_g_base[shown] == '@') ++shown;
            mem = call(s, mem, sub_6f7370, 0x6f7370, 0, {shown});
            if (readable(s->gpr.rax.dword)) shown = s->gpr.rax.dword;
        } else shown = g32(vd);
        snprintf(n, size, "%s", reinterpret_cast<char*>(bo2_g_base + shown));
    }
};

uint32_t load_font(State* s, void*& mem, uint32_t name) {
    mem = call(s, mem, sub_4274b0, 0x4274b0, 0x400, {name});
    mem = call(s, mem, sub_553910, 0x553910, 0x400, {s->gpr.rax.dword, 1u});
    return s->gpr.rax.dword;
}

void* hud_p2(State* s, void* mem, uint32_t sp) {
    const uint32_t gc = bot_gclient();
    if (!gc) return mem;
    const uint32_t rsp = s->gpr.rsp.dword, eax = s->gpr.rax.dword, frame = (sp - 0x400) & ~0xfu;
    static uint32_t big = 0, small = 0;
    if (!big) big = load_font(s, mem, kFontBig);
    if (!small) small = load_font(s, mem, kFontSmall);
    const uint32_t p1 = kEntities, gc0 = g32(p1 + 0x154);
    const uint32_t cg = g32(kCgPtr), snap = cg ? g32(cg + 0x28) : 0;
    const int round = snap ? (int32_t)g32(snap + 0x23b10) : 0;
    if (!big || !small || !gc0) { s->gpr.rax.dword = eax; return mem; }
    // Text calls run on a frame below the HUD's own scratch area (color, text).
    s->gpr.rsp.dword = frame;
    Hud h{s, mem, frame, frame + 0x80, frame + 0x100, big, small};
    auto right_aligned = [](const char* t, float edge, float hh) { return edge - 0.5f * hh * (float)strlen(t); };
    static const float white[4] = {1.f, 1.f, 1.f, 0.95f}, blue[4] = {0.55f, 0.75f, 1.f, 0.95f},
                       red[4] = {0.62f, 0.02f, 0.02f, 0.95f}, alert[4] = {1.f, 0.25f, 0.2f, 0.95f};
    auto player = [&](uint32_t ent, uint32_t other, int half, const float (&self)[4], const float (&them)[4]) {
        const uint32_t g = g32(ent + 0x154), go = g32(other + 0x154);
        if (!g) return;
        s->gpr.rsp.dword = frame;
        h.mem = place(s, h.mem, half);
        if (half && (g32(g + 0xc) & 0x80000) && g32(kWhite)) {   // red veil over player 2's half while down
            static const float veil[4] = {0.6f, 0.f, 0.f, 0.35f};
            memcpy(bo2_g_base + h.color, veil, sizeof veil);
            float pl[26];
            memcpy(pl, bo2_g_base + kPlace, sizeof pl);
            const float ex = pl[0] > 0.f ? pl[0] : 1.f, ey = pl[1] > 0.f ? pl[1] : 1.f;
            h.mem = call(s, h.mem, sub_545e90, 0x545e90, 0,
                         {kPlace, f2u((pl[14] - pl[20]) / ex), f2u((pl[15] - pl[21]) / ey), f2u(pl[16] / ex), f2u(pl[17] / ey),
                          1u, 1u, f2u(0.f), f2u(0.f), f2u(1.f), f2u(1.f), h.color, g32(kWhite)});
        }
        const uint32_t w = g32(g + 0x1b8);
        int k = 0;
        for (int i = 0; i < 15; ++i) if (g32(g + 0x264 + 0x1c * i) == w) { k = i; break; }
        char l[64];
        if (go) {
            snprintf(l, sizeof l, "%d", (int32_t)g32(go + 0x55c8));
            h.draw_text(big, l, right_aligned(l, -67.f, 19.f), -113.f, 3, 3, 19.f, them);
        }
        snprintf(l, sizeof l, "%d", (int32_t)g32(g + 0x55c8));
        h.draw_text(big, l, right_aligned(l, -67.f, 27.f), -80.f, 3, 3, 27.f, self);
        snprintf(l, sizeof l, "%d/%d", (int32_t)g32(g + 0x42c + 4 * k), (int32_t)g32(g + 0x3f0 + 4 * k));
        h.draw_text(big, l, right_aligned(l, -64.f, 35.f), -40.f, 3, 3, 35.f, white);
        const uint32_t lethal = g32(g + 0x1a4);
        int grenades = 0;
        for (int i = 0; i < 15; ++i) if (lethal && g32(g + 0x264 + 0x1c * i) == lethal) grenades = (int32_t)g32(g + 0x42c + 4 * i);
        char n[48];
        h.weapon_name(w, n, sizeof n);
        snprintf(l, sizeof l, "%s   GRENADES %d", n, grenades);
        h.draw_text(small, l, right_aligned(l, -64.f, 13.f), -16.f, 3, 3, 13.f, white, 3);
        snprintf(l, sizeof l, "%d", round);
        h.draw_text(big, l, 14.f, -14.f, 1, 3, 48.f, red);
        static const struct { int bit; const char* name; } perks[] = {
            {2, "Juggernog"}, {49, "Quick Revive"}, {20, "Speed Cola"}, {52, "Double Tap"}, {37, "Stamin-Up"},
            {25, "PhD Flopper"}, {0, "Mule Kick"}, {7, "Deadshot"}, {53, "Tombstone"}, {23, "Who's Who"},
            {28, "Electric Cherry"}, {41, "Vulture Aid"}};
        float yp = -66.f;
        for (const auto& pk : perks)
            if (g32(g + 0x548 + 4 * (pk.bit >> 5)) & 1u << (pk.bit & 31)) { h.draw_text(small, pk.name, 16.f, yp, 1, 3, 13.f, white, 3); yp -= 15.f; }
        // Interaction prompt of this player (ps +0x494 type, +0x498 string index), text from the game's own builder.
        const int32_t idx = (int32_t)g32(g + 0x498);
        if (cg && g32(g + 0x494) && idx > 0) {
            const uint32_t old = g32(cg + 0x69604), esi = s->gpr.rsi.dword;
            w32(cg + 0x69604, (uint32_t)idx);
            s->gpr.rsi.dword = 0;
            h.mem = call0(s, h.mem, sub_4dd100, 0x4dd100, 0);
            const uint32_t txt = s->gpr.rax.dword;
            w32(cg + 0x69604, old);
            s->gpr.rsi.dword = esi;
            if (readable(txt)) {
                char m[120];
                snprintf(m, sizeof m, "%s", reinterpret_cast<char*>(bo2_g_base + txt));
                h.draw_text(small, m, -0.25f * 15.f * (float)strlen(m), 100.f, 2, 2, 15.f, white, 3);
            }
        }
        h.draw_text(small, "-", -33.f, 6.f, 2, 2, 16.f, white, 3);   // crosshair
        h.draw_text(small, "-", 27.f, 6.f, 2, 2, 16.f, white, 3);
        h.draw_text(small, "|", -2.f, -24.f, 2, 2, 16.f, white, 3);
        h.draw_text(small, "|", -2.f, 36.f, 2, 2, 16.f, white, 3);
        if (g32(g + 0xc) & 0x80000) h.draw_text(big, "BLEEDING OUT", -66.f, 60.f, 2, 2, 22.f, alert);
        s->gpr.rsp.dword = frame;
        h.mem = place(s, h.mem, -1);
    };
    player(p1, g_bot, 0, white, blue);
    player(g_bot, p1, 1, blue, white);
    s->gpr.rsp.dword = rsp;
    s->gpr.rax.dword = eax;
    return h.mem;
}
}  // namespace

// The game's interaction prompt is player 1's in both halves: skipped, each player gets theirs in hud_p2.
extern "C" __attribute__((used)) void* bo2_ss_prompt(State* s, uint32_t pc, void* mem) {
    if (p2_live()) { s->gpr.rsp.dword += 4; return mem; }   // cdecl: just the ret
    return sub_4dd1b0_impl(s, pc, mem);
}

extern "C" __attribute__((used)) void* bo2_ss_hud2(State* s, uint32_t pc, void* mem) {
    const uint32_t sp = s->gpr.rsp.dword;
    mem = sub_4ad550_impl(s, pc, mem);
    if (on() && g_ready && !g_refused && g32(kClientState) == 10) mem = hud_p2(s, mem, sp);
    return mem;
}

// LUI: updated once per frame (skipped in pass B); a draw outside the two passes is player 1's: top half.
namespace {
void* lui_call(State* s, uint32_t pc, void* mem, void* (*impl)(State*, uint32_t, void*), bool is_draw) {
    if (!on()) return impl(s, pc, mem);
    if (!is_draw && g_pass_b) { s->gpr.rsp.dword += 4; return mem; }   // cdecl: just the ret
    if (is_draw && g_ready && !g_in_cg && g32(kClientState) == 10 && g32(kCgPtr)) {
        view(0); mem = place(s, mem, 0);
        mem = impl(s, pc, mem);
        view(-1); return place(s, mem, -1);
    }
    return impl(s, pc, mem);
}
}  // namespace
extern "C" __attribute__((used)) void* bo2_ss_lui_update(State* s, uint32_t pc, void* mem) {
    return lui_call(s, pc, mem, sub_9345b0_impl, false);
}
extern "C" __attribute__((used)) void* bo2_ss_lui_draw(State* s, uint32_t pc, void* mem) {
    return lui_call(s, pc, mem, sub_9315c0_impl, true);
}

// ---- the two passes ------------------------------------------------------------------------------------------------

namespace {
// In pass B the camera is in the bot's head: its body (centity 1) is lowered out of view for the pass.
void hide_bot(bool hide) {
    static float saved[4];
    static const uint32_t fields[4] = {0x2c, 0x174, 0x1f4, 0x338};
    if (!g_bot || (!hide && !saved[0])) return;
    const uint32_t ce = g32(kCgEntsPtr) + 0x380 + 8;
    for (int i = 0; i < 4; ++i) {
        if (hide) memcpy(&saved[i], bo2_g_base + ce + fields[i], 4);
        const float z = hide ? saved[i] - 50000 : saved[i];
        memcpy(bo2_g_base + ce + fields[i], &z, 4);
    }
    if (!hide) saved[0] = 0;
}

// The bot never gets a client-side body: give its clientInfo a character model and an anim tree, then build the DObj.
void* bot_body(State* s, void* mem) {
    static int tries = 0;
    if (!g_bot || tries >= 8 || g_passes < 64 || g_passes % 32) return mem;
    const uint32_t cg = g32(kCgPtr), cent = g32(kCgEntsPtr) + 0x380, n = bo2_g_base[cent + 0x2cf];
    const uint32_t ci = cg + 0x69a60 + 0x808 * n, ci1 = cg + 0x69a60;
    uint16_t dobj;
    memcpy(&dobj, bo2_g_base + kDobjHandles + 2, 2);
    if (dobj) { tries = 8; return mem; }
    char* name = reinterpret_cast<char*>(bo2_g_base + ci + 0xe0);
    if (!*name) {   // on TranZit, another of the four characters than player 1's; elsewhere player 1's
        static const char* const chars[4] = {"c_zom_player_oldman_fb", "c_zom_player_reporter_fb",
                                             "c_zom_player_engineer_fb", "c_zom_player_farmgirl_fb"};
        const char* p1 = reinterpret_cast<char*>(bo2_g_base + ci1 + 0xe0);
        bool transit = false;
        for (const char* c : chars) transit |= !strcmp(c, p1);
        strcpy(name, transit ? chars[!strcmp(chars[0], p1)] : p1);
    }
    const uint32_t eax = s->gpr.rax.dword;
    if (!g32(ci + 0x5c0)) {
        mem = call(s, mem, sub_787b80, 0x787b80, 0x700, {g32(g32(cg + 0x699c8) + 0x92f80), kAnimCallback});
        w32(ci + 0x5c0, s->gpr.rax.dword);
    }
    mem = call(s, mem, sub_8a9790, 0x8a9790, 0x700, {0u, 0u, cent + 0x1dc, ci, 0u});
    s->gpr.rax.dword = eax;
    memcpy(&dobj, bo2_g_base + kDobjHandles + 2, 2);
    fprintf(stderr, "[SPLIT-P2] body try %d: model \"%s\" -> DObj %u\n", ++tries, name, dobj);
    return mem;
}

// Scoreboard open: add valid clients missing from the list and rebuild the rows.
void* scores_row_p2(State* s, void* mem) {
    const uint32_t cg = g32(kCgPtr);
    if (!cg || !g32(cg + 0x69568)) return mem;
    const uint32_t list = cg + 0x688fc;
    uint32_t n = 0, added = 0;
    while (n < 18 && g32(list + 4 * n) <= 0x11) n++;
    for (uint32_t c = 0; c < 18 && n < 18; c++) {
        if (!g32(cg + 0x69a60 + 0x808 * c)) continue;
        bool seen = false;
        for (uint32_t i = 0; i < n && !seen; i++) seen = g32(list + 4 * i) == c;
        if (!seen) { w32(list + 4 * n++, c); added++; }
    }
    if (!added) return mem;
    const uint32_t eax = s->gpr.rax.dword;
    mem = call(s, mem, sub_4efc50, 0x4efc50, 0x700, {0u});
    s->gpr.rax.dword = eax;
    return mem;
}

// Down: health held at its value when falling (it otherwise regenerates and the bot gets up alone).
void hold_down() {
    static int fall_hp = 0;
    const uint32_t gc = bot_gclient();
    if (!gc) return;
    const int hp = (int32_t)g32(g_bot + 0x1a8);
    if (!(g32(gc + 0xc) & 0x80000)) { fall_hp = 0; return; }
    if (!fall_hp) fall_hp = std::max(1, hp);
    if (hp > fall_hp) w32(g_bot + 0x1a8, fall_hp);
}
}  // namespace

extern "C" __attribute__((used)) void* bo2_ss_draw(State* s, uint32_t pc, void* mem) {
    if (!on() || g_refused || s->gpr.rax.dword != 0 || g32(kClientState) != 10 || !g32(kCgPtr) ||
        !g32(g32(kCgPtr) + kRefdefW))
        return sub_55af10_impl(s, pc, mem);   // empty refdef = still loading
    if (!g_ready) {   // match start: player 2 joined in the lobby or not, decided here, once
        bo2_pad2_lock();
        if (!bo2_pad2_present()) {
            g_solo = true;
            fprintf(stderr, "[SPLIT] player 2 did not join: solo, full screen\n");
            return sub_55af10_impl(s, pc, mem);
        }
        fprintf(stderr, "[SPLIT] player 2 joined: split screen\n");
    }
    if (!g_ready && !(g_ready = prepare())) { g_refused = true; return sub_55af10_impl(s, pc, mem); }
    const uint32_t sp = s->gpr.rsp.dword, ret = g32(sp);

    // Player 2 joins once round 1 has begun: added during the round intro (round still 0), the game froze in 2 runs of 4.
    const uint32_t snap = g32(g32(kCgPtr) + 0x28);
    const bool round_on = snap && (int32_t)g32(snap + 0x23b10) >= 1;
    if (!g_bot && g_passes >= 64 && round_on && bo2_pad2_present()) {
        if (!g_bot_requested) {
            const uint32_t d = g32(kMaxClients);
            if (d && g32(d + 0x18) == 1) w32(d + 0x18, 2);
            g_bot_requested = true;   // served by the server frame (bo2_ss_sv)
        }
    }
    mem = bot_body(s, mem);
    mem = scores_row_p2(s, mem);
    // Back to one client slot after a while: with two, Escape no longer opens the solo pause menu.
    if (g_passes == 900 && bot_gclient() && g32(kEntities + 0x154) && g32(kMaxClients)) w32(g32(kMaxClients) + 0x18, 1);
    hold_down();

    view(0);
    g_in_cg = true;
    mem = place(s, mem, 0);
    mem = sub_55af10_impl(s, pc, mem);
    uint32_t r = s->gpr.rax.dword;
    {   // end A's command list like the end of a frame, keeping the global toggles
        const uint32_t parity = g32(kParity), light = g32(kLightToggle);
        mem = call0(s, mem, sub_41eb80, 0x41eb80, 0x120);
        w32(kParity, parity);
        w32(kLightToggle, light);
    }
    mem = call(s, mem, bo2_ss_send, 0x41c800, 0x120, {1u});
    // A's send clears the skinning cache; re-arm it like the start of a frame, or B's skinned models turn black.
    w32(kSkinCache, g32(g32(g32(kBackEnd) + 0x474198) + 0xc));
    w32(kSkinCacheOn, 1);

    view(1);
    s->gpr.rsp.dword = sp;
    w32(sp, ret);
    s->gpr.rax.dword = 0;
    g_pass_b = true;
    hide_bot(true);
    mem = sub_55af10_impl(s, pc, mem);
    hide_bot(false);
    g_pass_b = false;
    restore_camera();
    r |= s->gpr.rax.dword;
    view(-1);   // full screen for the rest of the frame (HUD, menus)
    mem = place(s, mem, -1);
    g_in_cg = false;

    s->gpr.rsp.dword = sp + 4;   // the second pass's ret
    s->gpr.rax.dword = r;
    if (pow2(++g_passes)) fprintf(stderr, "[SPLIT] double passes=%llu\n", (unsigned long long)g_passes);
    return mem;
}

// Frame view parms (refdef): player 1's eye is recorded in A; player 2's camera is set in B.
extern "C" __attribute__((used)) void* bo2_ss_vp(State* s, uint32_t pc, void* mem) {
    const uint32_t rd = g32(s->gpr.rsp.dword + 4);
    if (on() && !g_pass_b && rd == g32(kCgPtr) + kRefdef) {
        memcpy(g_aim_a, bo2_g_base + rd + kRdOrg, 12);
        memcpy(g_aim_a + 3, bo2_g_base + rd + kRdAxis, 12);
    }
    if (on()) put_camera(rd);
    return sub_41bb10_impl(s, pc, mem);
}

extern "C" __attribute__((used)) void* bo2_ss_refdef(State* s, uint32_t pc, void* mem) {
    const uint32_t rd = g32(s->gpr.rsp.dword + 4);
    if (on()) put_camera(rd);
    if (!on() || g_rd_set != rd) return sub_41bb80_impl(s, pc, mem);
    mem = sub_41bb80_impl(s, pc, mem);
    restore_camera();
    return mem;
}

// In B, player 1's view replays with player 2's camera: force third person so player 1's body is drawn.
extern "C" __attribute__((used)) void* bo2_ss_third(State* s, uint32_t pc, void* mem) {
    if (g_pass_b && p2_live()) w32(s->gpr.rsp.dword + 8, 1);
    return sub_522bf0_impl(s, pc, mem);
}

// Revive icon: the game skips the local client (cg+0); in B the local client is player 2.
extern "C" __attribute__((used)) void* bo2_ss_revive(State* s, uint32_t pc, void* mem) {
    const uint32_t cg = g32(kCgPtr);
    if (!g_pass_b || !p2_live() || !cg) return sub_4adaa0_impl(s, pc, mem);
    const uint32_t self = g32(cg);
    w32(cg, g32(g_bot) & 0x3ff);   // gentity+0 = entityState.number
    mem = sub_4adaa0_impl(s, pc, mem);
    w32(cg, self);
    return mem;
}

// Script hud elements come from the snapshot's playerState; in B they come from the bot's.
extern "C" __attribute__((used)) void* bo2_ss_hudelems(State* s, uint32_t pc, void* mem) {
    const uint32_t cg = g32(kCgPtr), gc = bot_gclient();
    if (!g_pass_b || !on() || !gc || !cg || !g32(cg + 0x28)) return sub_4ca660_impl(s, pc, mem);
    const uint32_t snap = g32(cg + 0x28);
    w32(cg + 0x28, gc - 0x10);
    mem = sub_4ca660_impl(s, pc, mem);
    w32(cg + 0x28, snap);
    return mem;
}

// ---- player 2's arms and weapon --------------------------------------------------------------------------------------

namespace {
// Player 2 keeps a copy of the view model state (entry 0 of [kVmArr], its DObj handle, cg+0x696c4 anim weapon and
// aim, and the per-weapon table [kVmWiArr]), swapped in for its own update.
// ponytail: the copy is dropped without freeing (one leaked slot of 0x800) when the bot changes.
constexpr uint32_t kVmWi = 0x2400, kVmHandle = kDobjHandles + 0x700 * 2, kVmWeap = 0x696c4;
uint8_t g_vm_p2[0xc0 + 2 + 8 + kVmWi];
void vm_swap(uint32_t cg) {
    static uint8_t t[sizeof g_vm_p2];
    uint8_t* const e = bo2_g_base + g32(kVmArr);
    uint8_t* const wi = bo2_g_base + g32(kVmWiArr);
    memcpy(t, e, 0xc0); memcpy(t + 0xc0, bo2_g_base + kVmHandle, 2); memcpy(t + 0xc2, bo2_g_base + cg + kVmWeap, 8);
    memcpy(t + 0xca, wi, kVmWi);
    memcpy(e, g_vm_p2, 0xc0); memcpy(bo2_g_base + kVmHandle, g_vm_p2 + 0xc0, 2);
    memcpy(bo2_g_base + cg + kVmWeap, g_vm_p2 + 0xc2, 8);
    memcpy(wi, g_vm_p2 + 0xca, kVmWi);
    memcpy(g_vm_p2, t, sizeof t);
}
bool vm_update_p2(State* s, void*& mem, uint32_t cg, uint32_t gc) {
    static uint32_t gc_before = 0;
    if (!g32(kVmArr) || !g32(kVmWiArr)) return false;
    if (gc != gc_before || !g32(g32(kVmArr))) {   // fresh copy, starting from player 1's weapon table
        memset(g_vm_p2, 0, sizeof g_vm_p2);
        memcpy(g_vm_p2 + 0xca, bo2_g_base + g32(kVmWiArr), kVmWi);
        gc_before = gc;
    }
    vm_swap(cg);
    const uint32_t eax = s->gpr.rax.dword;
    constexpr uint32_t kFrameMs = 0x48088;   // pass B redraws the same instant: lend pass A's frame time
    const uint32_t ms_b = g32(cg + kFrameMs);
    if (!ms_b && g_frame_ms_a) w32(cg + kFrameMs, g_frame_ms_a);
    mem = call(s, mem, sub_53c600, 0x53c600, 0x200, {0u, 0u}, kVmReturn);
    w32(cg + kFrameMs, ms_b);
    s->gpr.rax.dword = eax;
    return true;
}
}  // namespace

extern "C" __attribute__((used)) void* bo2_ss_arms(State* s, uint32_t pc, void* mem) {
    const uint32_t cg = g32(kCgPtr), gc = bot_gclient();
    if (cg && !g_pass_b) g_frame_ms_a = g32(cg + 0x48088);
    if (!on() || !g_pass_b || !gc || !cg) return sub_539990_impl(s, pc, mem);
    constexpr uint32_t kThird = 0x480a0, kPs = 0x480a8, kPsSize = 0x2a08, kViewAng = 0x648f0;
    static uint8_t ps_p1[kPsSize];
    const uint32_t third = g32(cg + kThird);
    memcpy(ps_p1, bo2_g_base + cg + kPs, kPsSize);
    memcpy(bo2_g_base + cg + kPs, bo2_g_base + gc, kPsSize);
    w32(cg + kThird, 0);
    const bool set = put_camera(cg + kRefdef);
    uint8_t ang_p1[12];
    memcpy(ang_p1, bo2_g_base + cg + kViewAng, 12);
    if (set || g_rd_set) {
        const float ang[3] = {g_cam_ang[0], g_cam_ang[1], 0.f};
        memcpy(bo2_g_base + cg + kViewAng, ang, 12);
    }
    const bool vm = vm_update_p2(s, mem, cg, gc);
    mem = sub_539990_impl(s, pc, mem);
    if (vm) vm_swap(cg);
    memcpy(bo2_g_base + cg + kViewAng, ang_p1, 12);
    if (set) restore_camera();
    w32(cg + kThird, third);
    memcpy(bo2_g_base + cg + kPs, ps_p1, kPsSize);
    return mem;
}

// Model lighting: each pass keeps its own lighting handle per model (the game resets it when the model is not visible
// in the current view, so a model seen by one half lost its lighting every frame). Also called from worker threads.
namespace {
std::unordered_map<uint32_t, uint16_t> g_light_bank[2];
std::mutex g_light_bank_mu;
}  // namespace
extern "C" __attribute__((used)) void* bo2_ss_light(State* s, uint32_t pc, void* mem) {
    if (!on() || !g_passes) return sub_457970_impl(s, pc, mem);
    const uint32_t hp = g32(s->gpr.rsp.dword + 0x10);
    if (!hp) return sub_457970_impl(s, pc, mem);
    uint16_t* h = reinterpret_cast<uint16_t*>(bo2_g_base + hp);
    const bool b = g_pass_b;
    {
        std::lock_guard<std::mutex> l(g_light_bank_mu);
        if (auto it = g_light_bank[b].find(hp); it != g_light_bank[b].end()) *h = it->second;
    }
    mem = sub_457970_impl(s, pc, mem);
    std::lock_guard<std::mutex> l(g_light_bank_mu);
    g_light_bank[b][hp] = *h;
    return mem;
}

// Add model to scene, view model (entity 0x3ff) only: in B its pose starts at player 1's eye: move it to player 2's;
// the skeleton request (+0x28) consumed in A is given again in B, and B gets its own pose copy.
extern "C" __attribute__((used)) void* bo2_ss_add(State* s, uint32_t pc, void* mem) {
    const uint32_t sp = s->gpr.rsp.dword, ent = g32(sp + 0xc) & 0xffff, pose = g32(sp + 8);
    if (!on() || ent != 0x3ff) return sub_4106f0_impl(s, pc, mem);
    if (g_pass_b && g_rd_set) {
        float o[3], p1[3], p2[3];
        memcpy(o, bo2_g_base + pose + 0x2c, 12);
        memcpy(p1, g_aim_a, 12);
        memcpy(p2, bo2_g_base + g_rd_set + kRdOrg, 12);
        const float d[3] = {o[0] - p1[0], o[1] - p1[1], o[2] - p1[2]};
        if (d[0] * d[0] + d[1] * d[1] + d[2] * d[2] < 64.f * 64.f) {   // the pose does start at player 1's eye
            const float n[3] = {p2[0] + d[0], p2[1] + d[1], p2[2] + d[2]};
            memcpy(bo2_g_base + pose + 0x2c, n, 12);
        }
    }
    static uint32_t calc_a = 0;
    const uint32_t calc = g32(pose + 0x28);
    if (!g_pass_b) calc_a = calc;
    else if (!calc && calc_a) w32(pose + 0x28, calc_a);
    static const uint32_t pose_b = win32_shim_alloc(0x80, 16);
    if (g_pass_b && pose_b) { memcpy(bo2_g_base + pose_b, bo2_g_base + pose, 0x80); w32(sp + 8, pose_b); }
    return sub_4106f0_impl(s, pc, mem);
}
