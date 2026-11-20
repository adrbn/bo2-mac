#!/usr/bin/env python3
"""check.py sps|mps|zms [--seeds N] [--insn] [--jobs J] [VA ...]

Checks the blocks of tools/interp_vas_<mode>.txt, translated as the game gets them (tools/steam/interp_lift.sh: same
lift, same inlined accessors), against the Unicorn interpreter the game ran them through. Off the game: nothing
is launched. Each block runs from N random states (registers, flags, xmm, a 1 MB random area the registers point
into, the game image) once translated (harness.cpp) and once in Unicorn. Then the registers, eip, the flags the
last instructions leave defined (capstone), xmm0-7 and every byte either side wrote are compared.
--insn also lifts every instruction of every block alone and checks it the same way, so the instructions random
paths never reach are covered too. Prints one line per block (or instruction) that differs, then a summary.
Work files: boot_cache/<mode>/interp_check (lifts are reused, delete it after a lift change).
"""
import argparse, ctypes, os, random, struct, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
L = os.path.join(os.environ.get("LLVM17", "/opt/homebrew/opt/llvm@17"), "bin")
# Same flags as tools/steam/interp_lift.sh: the checked code is the shipped code.
LIFT_ENV = dict(LIFT_PISTE_ENTIERE="1", BO2_INLINE_FLAGS="1", BO2_INLINE_TRIV="1", BO2_INLINE_READS="1",
                BO2_INLINE_WRITES="1")
SCR, SCR_N = 0x20000000, 0x100000          # the random area
LIMIT = 200000                             # Unicorn instructions before a run counts as a loop
FLAG_BITS = dict(CF=0, PF=2, AF=4, ZF=6, SF=7, DF=10, OF=11)
FLAG_MASK = sum(1 << b for b in FLAG_BITS.values())
WHY = __import__('collections').Counter()   # why runs were skipped
KINDS = {0: "?", 1: "call", 2: "ret", 3: "jump", 4: "missing", 5: "error", 6: "icall", 7: "timeout"}


class IO(ctypes.Structure):
    _fields_ = [("gpr", ctypes.c_uint32 * 8), ("eip", ctypes.c_uint32), ("eflags", ctypes.c_uint32),
                ("xmm", (ctypes.c_uint8 * 16) * 8), ("kind", ctypes.c_uint32), ("rip", ctypes.c_uint32)]


def sh(cmd, **kw):
    r = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if r.returncode:
        sys.exit(f"{cmd[0]} failed: {(r.stderr or r.stdout)[-800:]}")
    return r.stdout


# ── translation ────────────────────────────────────────────────────────────────────────────────────────────────
def lift(mode, specs, wd, jobs):
    """Lifts each spec into wd (tools/lift_extra.py), in `jobs` processes. Returns the VAs that lifted."""
    os.makedirs(wd, exist_ok=True)
    todo = [s for s in specs if not os.path.exists(f"{wd}/func_{s.split(':')[0]}.bc.fixed")]
    env = dict(os.environ, BO2_TARGET=mode, BOOT_WORKDIR=f"boot_cache/{mode}", **LIFT_ENV)
    code = ("import sys; sys.path.insert(0, 'tools'); import lift_extra; lift_extra.WORKDIR = sys.argv[1]; "
            "lift_extra.lift(sys.argv[2:])")
    lots = [todo[i::jobs] for i in range(jobs)] if todo else []
    with ThreadPoolExecutor(jobs) as ex:
        list(ex.map(lambda lot: subprocess.run([sys.executable, "-c", code, wd] + lot, cwd=ROOT, env=env,
                                               capture_output=True), [l for l in lots if l]))
    for f in os.listdir(wd):
        if f.endswith(".bc"):
            os.remove(os.path.join(wd, f))   # remill's raw output: only .bc.fixed is used
    return [s.split(":")[0] for s in specs if os.path.exists(f"{wd}/func_{s.split(':')[0]}.bc.fixed")]


def objects(wd, vas, prefix, jobs):
    """Each func_<va>.bc.fixed compiled as the game's are (-O2), its entry renamed <prefix>_<va>."""
    def one(va):
        o = f"{wd}/{va}.o"
        if not os.path.exists(o):
            sh([f"{L}/clang", "-arch", "arm64", "-c", "-O2", "-x", "ir", "-o", o + ".tmp", f"{wd}/func_{va}.bc.fixed"])
            sh([f"{L}/llvm-objcopy", f"--redefine-sym=_sub_{va}=_{prefix}_{va}", o + ".tmp", o])
            os.remove(o + ".tmp")
        return o
    with ThreadPoolExecutor(jobs) as ex:
        return list(ex.map(one, vas))


def build(mode, objs, out):
    """harness.cpp + the objects + an exit for every other function they call -> a dylib."""
    d = os.path.dirname(out)
    und = set()
    for i in range(0, len(objs), 500):
        und |= {s for s in sh([f"{L}/llvm-nm", "-u", "-j"] + objs[i:i + 500]).split() if s.startswith("_sub_")}
    with open(f"{d}/exits.cpp", "w") as f:
        f.write('#include <cstdint>\nextern "C" [[noreturn]] void chk_exit(uint32_t, uint32_t);\n')
        for s in sorted(und):
            f.write(f'extern "C" void* {s[1:]}(void*, uint32_t pc, void*) {{ chk_exit(1, pc); }}\n')
    with open(f"{d}/objs.rsp", "w") as f:
        f.write("\n".join(objs))
    rem = os.environ.get("BO2_TOOLCHAIN") and os.path.join(os.environ["BO2_TOOLCHAIN"], "remill") \
        or os.path.join(ROOT, "tools", "remill-src")
    sh([f"{L}/clang++", "-arch", "arm64", "-shared", "-O1", "-std=c++17", "-DADDRESS_SIZE_BITS=32",
        "-I", os.path.join(rem, "include"), "-o", out, os.path.join(ROOT, "tools/interp_check/harness.cpp"),
        f"{d}/exits.cpp", f"@{d}/objs.rsp"])


# ── the game image ──────────────────────────────────────────────────────────────────────────────────────────────
def image(mode):
    """[(va, bytes, writable)] of the PE headers and sections, and the sweep that gives a block its extent."""
    sys.path.insert(0, os.path.join(ROOT, "tools"))
    os.environ["BO2_TARGET"] = mode
    import boot, lift_roots
    from pe_parser import PEFile, SEC_WRITE
    pe = PEFile(boot.BIN).parse()
    secs = [(pe.image_base, pe._raw[:0x1000], False)]
    for s in pe.sections:
        raw = pe._raw[s.pointer_to_raw_data:s.pointer_to_raw_data + s.size_of_raw_data]
        size = max(s.virtual_size, len(raw))
        secs.append((pe.image_base + s.virtual_address, raw.ljust(size, b"\0")[:size],
                     bool(s.characteristics & SEC_WRITE)))
    return secs, lift_roots._Closure(pe, {})


# ── the interpreter ─────────────────────────────────────────────────────────────────────────────────────────────
class Emu:
    def __init__(self, secs):
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_UNMAPPED
        from unicorn import x86_const as X
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_GRP_JUMP, CS_GRP_CALL, CS_GRP_RET, x86_const as C
        self.X, self.C = X, C
        self.grp = (CS_GRP_JUMP, CS_GRP_CALL, CS_GRP_RET)
        self.md = Cs(CS_ARCH_X86, CS_MODE_32); self.md.detail = True
        self.uc = uc = Uc(UC_ARCH_X86, UC_MODE_32)
        self.regs = [X.UC_X86_REG_EAX, X.UC_X86_REG_ECX, X.UC_X86_REG_EDX, X.UC_X86_REG_EBX, X.UC_X86_REG_ESP,
                     X.UC_X86_REG_EBP, X.UC_X86_REG_ESI, X.UC_X86_REG_EDI]
        self.xmm = [getattr(X, f"UC_X86_REG_XMM{i}") for i in range(8)]
        self.lo = min(v for v, _, _ in secs) & ~0xfff
        hi = (max(v + len(b) for v, b, _ in secs) + 0xfff) & ~0xfff
        self.img = bytearray(hi - self.lo)
        for v, b, _ in secs:
            self.img[v - self.lo:v - self.lo + len(b)] = b
        uc.mem_map(self.lo, hi - self.lo); uc.mem_write(self.lo, bytes(self.img))
        uc.mem_map(SCR, SCR_N)
        self.hi, self.extra, self.writes, self.dec = hi, set(), [], {}
        uc.hook_add(UC_HOOK_MEM_UNMAPPED, self._unmapped)
        self.ro = [(v, v + len(b)) for v, b, w in secs if not w]
        uc.hook_add(UC_HOOK_MEM_WRITE, self._write)
        uc.hook_add(UC_HOOK_CODE, self._code)
        fl = {}
        for f in FLAG_BITS:
            fl[f] = (getattr(C, f"X86_EFLAGS_UNDEFINED_{f}", 0),
                     sum(getattr(C, f"X86_EFLAGS_{k}_{f}", 0) for k in ("MODIFY", "RESET", "SET")))
        self.fl = fl

    def _write(self, uc, access, addr, size, value, data):
        self.writes.append((addr, size))
        if any(lo <= addr < hi for lo, hi in self.ro):
            self.bad = "writes to a read-only section (the game would fault)"; uc.emu_stop()

    def _unmapped(self, uc, access, addr, size, value, data):
        for p in range(addr & ~0xfff, (addr + size + 0xfff) & ~0xfff, 0x1000):
            if p not in self.extra and not (self.lo <= p < self.hi) and not (SCR <= p < SCR + SCR_N):
                uc.mem_map(p, 0x1000); self.extra.add(p)
        return True

    def insn(self, addr):
        i = self.dec.get(addr)
        if i is None:
            i = next(self.md.disasm(bytes(self.uc.mem_read(addr, 16)), addr), None)
            self.dec[addr] = i
        return i

    def _code(self, uc, addr, size, data):
        if self.n:
            if (addr == self.stop_at or not (self.ext[0] <= addr < self.ext[1]) or self.breaks
                    or (self.single and not self.rep)):
                uc.emu_stop(); return
        self.n += 1
        i = self.insn(addr)
        if i is None:
            uc.emu_stop(); self.bad = "undecodable"; return
        self.rep = self.single and i.prefix[0] in (0xf2, 0xf3) and addr == self.ext[0]
        for f, (u, m) in self.fl.items():
            b = 1 << FLAG_BITS[f]
            if i.eflags & u: self.undef |= b
            elif i.eflags & m: self.undef &= ~b
        self.undef |= undefined_beyond_capstone(i, self.C)
        self.breaks = False
        if any(i.group(g) for g in self.grp):
            ops = i.operands
            direct = ops and ops[0].type == self.C.X86_OP_IMM
            if i.group(self.grp[2]) or not direct:
                self.breaks = True                     # ret, indirect jmp/call: the translation always leaves
            elif i.group(self.grp[1]) and ops[0].imm != addr + i.size:
                self.breaks = True                     # a direct call (not `call $+5`) leaves to the callee

    def restore(self):
        for a, n in self.writes:
            for p in range(a, a + n):
                p &= 0xffffffff   # an access across 4 GB wraps
                self.uc.mem_write(p, bytes([self.pristine(p)]))
        self.writes = []

    def pristine(self, a):
        if self.lo <= a < self.hi: return self.img[a - self.lo]
        if SCR <= a < SCR + SCR_N: return self.scr[a - SCR]
        return 0

    def run(self, ext, st, scr, stop_at, single):
        X, uc = self.X, self.uc
        self.restore(); self.scr = scr; uc.mem_write(SCR, scr)
        for r, v in zip(self.regs, st["gpr"]): uc.reg_write(r, v)
        uc.reg_write(X.UC_X86_REG_EFLAGS, st["eflags"] | 2)
        for r, v in zip(self.xmm, st["xmm"]): uc.reg_write(r, int.from_bytes(v, "little"))
        uc.reg_write(X.UC_X86_REG_FPCW, 0x27F); uc.reg_write(X.UC_X86_REG_FPSW, 0); uc.reg_write(X.UC_X86_REG_FPTAG, 0xffff)
        uc.reg_write(X.UC_X86_REG_MXCSR, 0x1F80)
        self.ext, self.stop_at, self.single = ext, stop_at, single
        self.n, self.breaks, self.rep, self.undef, self.bad = 0, False, False, 0, None
        try:
            uc.emu_start(ext[0], 0xffffffff, count=LIMIT)
        except Exception as e:
            return None, f"trap {e}"
        if self.bad: return None, self.bad
        if self.n >= LIMIT: return None, "loop"
        return dict(gpr=[uc.reg_read(r) for r in self.regs], eip=uc.reg_read(X.UC_X86_REG_EIP),
                    eflags=uc.reg_read(X.UC_X86_REG_EFLAGS) & FLAG_MASK,
                    xmm=[uc.reg_read(r).to_bytes(16, "little") for r in self.xmm]), None


SHIFTS = {"shl", "sal", "shr", "sar", "rol", "ror", "rcl", "rcr", "shld", "shrd"}


def undefined_beyond_capstone(i, C):
    """Flags the Intel manual leaves undefined that capstone reports as written: OF after a shift or rotate whose
    count is not 1, SF ZF AF PF after mul / imul."""
    m = i.mnemonic
    if m in SHIFTS:
        n = i.operands[-1]
        if len(i.operands) < 2 or n.type != C.X86_OP_IMM or n.imm != 1:
            return 1 << FLAG_BITS["OF"]
    if m in ("mul", "imul"):
        return sum(1 << FLAG_BITS[f] for f in ("SF", "ZF", "AF", "PF"))
    return 0


# ── one run, both sides ─────────────────────────────────────────────────────────────────────────────────────────
def state(rng, consts=()):
    """Random registers, flags, xmm and area. `consts`: the block's immediates, so its compares see both sides."""
    def val():
        k = rng.random()
        if consts and k < .3: return rng.choice(consts)
        if k < .4: return SCR + rng.randrange(0x1000, SCR_N - 0x1000) & ~3
        if k < .7: return rng.randrange(65)
        return rng.getrandbits(32)
    gpr = [val() for _ in range(8)]
    gpr[4] = SCR + 0x80000 - 4 * rng.randrange(256)
    fl = sum(1 << b for f, b in FLAG_BITS.items() if f != "DF" and rng.random() < .5)
    xmm = []
    for _ in range(8):
        if rng.random() < .5:
            xmm.append(b"".join(struct.pack("<f", rng.uniform(-1000, 1000)) for _ in range(4)))
        else:
            xmm.append(rng.randbytes(16))
    scr = bytearray(rng.randbytes(SCR_N))
    for _ in range(SCR_N // 64):    # some dwords are pointers back into the area
        o = rng.randrange(0, SCR_N, 4)
        scr[o:o + 4] = (SCR + rng.randrange(0, SCR_N - 0x100) & ~3).to_bytes(4, "little")
    return dict(gpr=gpr, eflags=fl, xmm=xmm), bytes(scr)


class Native:
    def __init__(self, dylib, secs):
        self.lib = lib = ctypes.CDLL(dylib)
        lib.chk_diff.restype = ctypes.c_uint32
        self.secs = secs
        self.wsecs = [(v, b, ctypes.create_string_buffer(b, len(b))) for v, b, w in secs if w]
        self.out = (ctypes.c_uint32 * 1000000)()
        self.dirty = []
        self.fresh()

    def fresh(self):
        if self.lib.chk_init() if not hasattr(self, "ready") else self.lib.chk_reset():
            sys.exit("harness: no 4 GB guest memory")
        self.ready = True
        for v, b, _ in self.secs:
            self.lib.chk_put(v, b, len(b))

    def fn(self, name):
        return ctypes.cast(getattr(self.lib, name), ctypes.c_void_p)

    def run(self, fn, va, st, scr, scr_buf):
        lib = self.lib
        lib.chk_put(SCR, scr, SCR_N)
        io = IO()
        io.gpr[:] = st["gpr"]; io.eip = va; io.eflags = st["eflags"]
        for i, x in enumerate(st["xmm"]): ctypes.memmove(io.xmm[i], x, 16)
        lib.chk_run(fn, ctypes.byref(io))
        written = set()
        for v, b, buf in self.wsecs:
            k = lib.chk_diff(v, len(b), buf, self.out, len(self.out))
            written.update(self.out[:k])
            self.dirty += [(a, b[a - v]) for a in self.out[:k]]
        k = lib.chk_diff(SCR, SCR_N, scr_buf, self.out, len(self.out))
        written.update(self.out[:k])
        return dict(gpr=list(io.gpr), eip=io.eip, eflags=io.eflags & FLAG_MASK, kind=io.kind, rip=io.rip,
                    xmm=[bytes(io.xmm[i]) for i in range(8)]), written

    def undo(self):
        """The image bytes the last run wrote, back to the image's (after the comparison)."""
        for a, b in self.dirty: self.lib.chk_put(a, bytes([b]), 1)
        self.dirty = []

    def byte(self, a):
        b = ctypes.create_string_buffer(1); self.lib.chk_get(a, b, 1); return b.raw[0]


NAMES = ["eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"]


def compare(nat, emu, native, interp, undef, written):
    d = [f"{n} {a:#x}/{b:#x}" for n, a, b in zip(NAMES, native["gpr"], interp["gpr"]) if a != b]
    if native["eip"] != interp["eip"]:
        d.append(f"eip {native['eip']:#x}/{interp['eip']:#x} ({KINDS.get(native['kind'])})")
    fx = (native["eflags"] ^ interp["eflags"]) & ~undef
    d += [f"{f}={native['eflags'] >> b & 1}/{interp['eflags'] >> b & 1}" for f, b in FLAG_BITS.items() if fx >> b & 1]
    d += [f"xmm{i} {a.hex()}/{b.hex()}" for i, (a, b) in enumerate(zip(native["xmm"], interp["xmm"])) if a != b]
    addrs = set(written)
    for a, n in emu.writes:
        addrs.update(p & 0xffffffff for p in range(a, a + n))
    bad = [a for a in sorted(addrs) if nat.byte(a) != emu.uc.mem_read(a, 1)[0]]
    if bad:
        d.append(f"memory {len(bad)} bytes from {bad[0]:#x}")
    return d


def immediates(emu, ext):
    """Each immediate of the extent's instructions, and its neighbours (32-bit)."""
    from capstone import x86_const as C
    out, a = set(), ext[0]
    while a < ext[1]:
        i = emu.insn(a)
        if i is None: break
        for o in i.operands:
            if o.type == C.X86_OP_IMM:
                out.update((o.imm + d) & 0xffffffff for d in (-1, 0, 1))
        a += i.size
    return sorted(out)


def check(nat, emu, fn, va, ext, seeds, single, label):
    """Runs one translated entry from `seeds` random states. Returns (runs compared, skipped, first difference)."""
    ok = skip = 0
    consts = immediates(emu, ext)
    for seed in range(seeds):
        rng = random.Random(f"{va:x}:{seed}")
        st, scr = state(rng, consts)
        scr_buf = ctypes.create_string_buffer(scr, SCR_N)
        native, written = nat.run(fn, va, st, scr, scr_buf)
        stop_at = native["eip"] if native["kind"] in (4, 5) and ext[0] <= native["eip"] < ext[1] else None
        interp, why = emu.run(ext, st, scr, stop_at, single)
        if interp is None or native["kind"] == 7:
            skip += 1; WHY[why or "translation timed out"] += 1
            nat.dirty = []; nat.fresh()   # the translation may have written on where the interpreter stopped
        else:
            d = compare(nat, emu, native, interp, emu.undef, written)
            if d:
                nat.dirty = []; nat.fresh()
                return ok, skip, f"{label} seed {seed}: " + "; ".join(d[:6])
            ok += 1
        nat.undo()
        # pages the interpreter mapped and wrote: zero again on the translated side as well
        for a, n in emu.writes:
            if not (emu.lo <= a < emu.hi or SCR <= a < SCR + SCR_N):
                for p in range(a, a + n): nat.lib.chk_put(p & 0xffffffff, b"\0", 1)
    return ok, skip, None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mode", choices=["sps", "mps", "zms"])
    ap.add_argument("vas", nargs="*")
    ap.add_argument("--seeds", type=int, default=200)
    ap.add_argument("--insn", action="store_true")
    ap.add_argument("--jobs", type=int, default=2)
    a = ap.parse_args()
    os.chdir(ROOT)
    mode, wd = a.mode, f"boot_cache/{a.mode}/interp_check"
    vas = a.vas or [l.split("#")[0].split()[0].lower().removeprefix("0x")
                    for l in open(f"tools/interp_vas_{mode}.txt") if l.split("#")[0].strip()]
    secs, sweep = image(mode)
    ext = {va: (int(va, 16), int(va, 16) + sweep._decode(int(va, 16))[0]) for va in vas}
    blocks = lift(mode, vas, f"{wd}/blk", a.jobs)
    objs = objects(f"{wd}/blk", blocks, "blk", a.jobs)
    insns = {}
    if a.insn:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32
        md, img = Cs(CS_ARCH_X86, CS_MODE_32), Emu(secs)
        for va in blocks:
            lo, hi = ext[va]
            for i in md.disasm(bytes(img.img[lo - img.lo:hi - img.lo]), lo):
                insns[f"{i.address:x}"] = (i.address, i.address + i.size, f"{i.mnemonic} {i.op_str}")
        print(f"{len(insns)} instructions to lift", flush=True)
        done = lift(mode, [f"{v}:{e - s:x}" for v, (s, e, _) in insns.items()], f"{wd}/ins", a.jobs)
        objs += objects(f"{wd}/ins", done, "ins", a.jobs)
        insns = {v: insns[v] for v in done}
    dylib = f"{wd}/check_{mode}.dylib"
    build(mode, objs, dylib)
    nat, emu = Native(dylib, secs), Emu(secs)
    fails = []
    tot = [0, 0]
    for va in vas:
        if va not in blocks:
            fails.append(f"block {va}: did not lift"); continue
        ok, skip, d = check(nat, emu, nat.fn(f"blk_{va}"), int(va, 16), ext[va], a.seeds, False, f"block {va}")
        tot[0] += ok; tot[1] += skip
        print(f"block {va}: {ok} identical, {skip} skipped" + (f"\n  {d}" if d else ""), flush=True)
        if d: fails.append(d)
    n_ins = 0
    for v, (s, e, text) in sorted(insns.items()):
        ok, skip, d = check(nat, emu, nat.fn(f"ins_{v}"), s, (s, e), max(a.seeds // 10, 20), True,
                            f"insn {v} {text}")
        n_ins += 1; tot[0] += ok; tot[1] += skip
        if d:
            fails.append(d); print(f"  {d}", flush=True)
    print(f"\n{mode}: {len(vas)} blocks, {n_ins} instructions, {tot[0]} runs identical, {tot[1]} skipped, "
          f"{len(fails)} differ")
    for why, n in WHY.most_common():
        print(f"  skipped {n}: {why}")
    for f in fails:
        print(f"  {f}")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
