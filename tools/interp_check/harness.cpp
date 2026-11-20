// harness.cpp: runs one translated block (or one translated instruction) on a State and a flat 4 GB guest memory,
// for tools/interp_check/check.py. The block objects are the ones tools/steam/interp_lift.sh joins to the game: same
// lift, same inlined accessors, so the globals those accessors read are defined here with their fast-path values.
// Every way out of a block (a call to another function, a return, a jump, a missing block, an error) ends the run.
#include "remill/Arch/X86/Runtime/State.h"
#include <csetjmp>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <sys/mman.h>
#include <sys/time.h>

extern "C" {
uint8_t* bo2_g_base;
uint8_t* bo2_w32_slow_map;          // all zero: no guest page takes the slow path
uint32_t bo2_w32_cold, bo2_w64_cold;
int g_park_pending;
uint64_t g_guest_writes, g_w32_preempt_left, g_w32_preempt_every, bo2_sched_tp;   // preempt_every 0: no preemption
bool g_any_hosted;
void win32_thread_write_tick() {}
}

namespace {
constexpr uint64_t kGuest = (1ull << 32) + 4096;   // + page 0 again: an access across 4 GB wraps, as on x86
sigjmp_buf g_out;
uint32_t g_kind, g_pc;
[[noreturn]] void leave(uint32_t kind, uint32_t pc) { g_kind = kind; g_pc = pc; siglongjmp(g_out, 1); }
void on_alarm(int) { leave(7, 0); }
template <typename T> T rd(uint32_t a) { T v; memcpy(&v, bo2_g_base + a, sizeof v); return v; }
template <typename T> void* wr(void* m, uint32_t a, T v) { memcpy(bo2_g_base + a, &v, sizeof v); return m; }
}

// Exit kinds, read by check.py: 1 call to another function, 2 return, 3 jump, 4 missing block, 5 error,
// 6 indirect call, 7 timeout.
extern "C" [[noreturn]] void chk_exit(uint32_t kind, uint32_t pc) { leave(kind, pc); }
extern "C" void* __remill_function_return(State&, uint32_t pc, void*) { leave(2, pc); }
extern "C" void* __remill_jump(State&, uint32_t pc, void*) { leave(3, pc); }
extern "C" void* __remill_missing_block(State&, uint32_t pc, void*) { leave(4, pc); }
extern "C" void* __remill_error(State&, uint32_t pc, void*) { leave(5, pc); }
extern "C" void* __remill_function_call(State&, uint32_t pc, void*) { leave(6, pc); }
extern "C" void* __remill_atomic_begin(void* m) { return m; }
extern "C" void* __remill_atomic_end(void* m) { return m; }

#define MEM(n, T)                                                                                  \
  extern "C" T __remill_read_memory_##n(void*, uint32_t a) { return rd<T>(a); }                    \
  extern "C" T __remill_read_memory_##n##_slow(void*, uint32_t a) { return rd<T>(a); }             \
  extern "C" void* __remill_write_memory_##n(void* m, uint32_t a, T v) { return wr(m, a, v); }     \
  extern "C" void* __remill_write_memory_##n##_slow(void* m, uint32_t a, T v) { return wr(m, a, v); }
MEM(8, uint8_t) MEM(16, uint16_t) MEM(32, uint32_t) MEM(64, uint64_t) MEM(f32, float) MEM(f64, double)

struct IO {
  uint32_t gpr[8];      // eax ecx edx ebx esp ebp esi edi
  uint32_t eip, eflags;
  uint8_t xmm[8][16];
  uint32_t kind, rip;   // out: how the block ended, and the State's pc then
};

using Block = void* (*)(State*, uint32_t, void*);

extern "C" int chk_init() {
  void* p = mmap(nullptr, kGuest, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE | MAP_NORESERVE, -1, 0);
  if (p == MAP_FAILED) return -1;
  bo2_g_base = static_cast<uint8_t*>(p);
  mach_vm_address_t top = reinterpret_cast<mach_vm_address_t>(p) + (1ull << 32);
  vm_prot_t cur, max;
  if (mach_vm_remap(mach_task_self(), &top, 4096, 0, VM_FLAGS_FIXED | VM_FLAGS_OVERWRITE, mach_task_self(),
                    reinterpret_cast<mach_vm_address_t>(p), FALSE, &cur, &max, VM_INHERIT_NONE) != KERN_SUCCESS)
    return -1;
  static uint8_t slow_map[1 << 16];
  bo2_w32_slow_map = slow_map;
  signal(SIGALRM, on_alarm);
  return 0;
}

// A fresh guest memory (after a run that wrote somewhere check.py does not look).
extern "C" int chk_reset() {
  munmap(bo2_g_base, kGuest);
  return chk_init();
}

extern "C" void chk_put(uint32_t a, const uint8_t* src, uint32_t n) { memcpy(bo2_g_base + a, src, n); }
extern "C" void chk_get(uint32_t a, uint8_t* dst, uint32_t n) { memcpy(dst, bo2_g_base + a, n); }

// Addresses in [lo, lo+n) whose byte differs from ref; at most max written to out. Returns the count found.
extern "C" uint32_t chk_diff(uint32_t lo, uint32_t n, const uint8_t* ref, uint32_t* out, uint32_t max) {
  uint32_t k = 0;
  for (uint32_t i = 0; i < n; i += 8) {
    const uint32_t len = n - i < 8 ? n - i : 8;
    if (!memcmp(bo2_g_base + lo + i, ref + i, len)) continue;
    for (uint32_t j = i; j < i + len; ++j)
      if (bo2_g_base[lo + j] != ref[j] && k < max) out[k++] = lo + j;
  }
  return k;
}

extern "C" void chk_run(Block fn, IO* io) {
  alignas(64) static State s;
  memset(&s, 0, sizeof s);
  Reg* const r[8] = {&s.gpr.rax, &s.gpr.rcx, &s.gpr.rdx, &s.gpr.rbx, &s.gpr.rsp, &s.gpr.rbp, &s.gpr.rsi, &s.gpr.rdi};
  for (int i = 0; i < 8; ++i) r[i]->dword = io->gpr[i];
  for (int i = 0; i < 8; ++i) memcpy(&s.vec[i], io->xmm[i], 16);
  const uint32_t f = io->eflags;
  s.aflag.cf = f & 1; s.aflag.pf = f >> 2 & 1; s.aflag.af = f >> 4 & 1; s.aflag.zf = f >> 6 & 1;
  s.aflag.sf = f >> 7 & 1; s.aflag.df = f >> 10 & 1; s.aflag.of = f >> 11 & 1;
  s.gpr.rip.dword = io->eip;
  s.x87.fxsave.cwd.flat = 0x027F;   // as the runtime sets each guest thread (kernel32_impl.cpp)
  s.x87.fxsave.mxcsr.flat = 0x1F80;
  g_kind = 0; g_pc = 0;
  itimerval t{}; t.it_value.tv_usec = 200000;
  if (!sigsetjmp(g_out, 1)) {
    setitimer(ITIMER_REAL, &t, nullptr);
    fn(&s, io->eip, nullptr);
    g_kind = 2; g_pc = s.gpr.rip.dword;   // a plain return: the block's own `ret` path
  }
  t = {}; setitimer(ITIMER_REAL, &t, nullptr);
  for (int i = 0; i < 8; ++i) io->gpr[i] = r[i]->dword;
  for (int i = 0; i < 8; ++i) memcpy(io->xmm[i], &s.vec[i], 16);
  io->eflags = (s.aflag.cf & 1) | (s.aflag.pf & 1) << 2 | (s.aflag.af & 1) << 4 | (s.aflag.zf & 1) << 6 |
               (s.aflag.sf & 1) << 7 | (s.aflag.df & 1) << 10 | (s.aflag.of & 1) << 11;
  io->kind = g_kind; io->eip = g_pc; io->rip = s.gpr.rip.dword;
}
