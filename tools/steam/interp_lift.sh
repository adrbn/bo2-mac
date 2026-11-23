#!/bin/zsh
# interp_lift.sh sps|mps|zms : translates ahead of time the blocks the game ran through the interpreter
# (tools/interp_vas_<mode>.txt, plus the runs/interp_vas_<mode>.txt that play_steam.sh collects). Each VA is lifted alone
# (tools/lift_extra.py), joined to lifted.o and added to bc_files.txt, so the relink registers it: seconds, not a relift.
# A VA lifted.o already defines is left to it. A call to a function lifted.o lacks goes through the runtime's dispatcher
# (translated if it is, interpreter if not). A VA that fails to lift stays interpreted. Run after native_impl.sh.
set -e
cd "$(dirname $0)/../.."
T=$1; W=boot_cache/$T/interp; L=${LLVM17:-/opt/homebrew/opt/llvm@17}/bin; O=boot_cache/$T/lifted.o
[[ $T == (sps|mps|zms) ]] || { echo "usage: interp_lift.sh sps|mps|zms"; exit 1 }
mkdir -p $W
$L/llvm-nm --defined-only -j $O | sed -n 's/^_sub_\([0-9a-f]*\)$/\1/p' | sort -u > $W/have.txt
cat tools/interp_vas_$T.txt runs/interp_vas_$T.txt 2>/dev/null | sed 's/#.*//; s/^[[:space:]]*0[xX]//; s/[[:space:]]//g' \
  | tr A-F a-f | grep -E '^[0-9a-f]+$' | sort -u | comm -23 - $W/have.txt > $W/todo.txt
[ -s $W/todo.txt ] || { echo "interp_lift: nothing to add to $O"; exit 0; }
# A lifted.o without them is a new one: lift again rather than reuse what an older lift left (tools/boot.py, lift_one).
find $W -name 'func_*' -delete
# LIFT_PISTE_ENTIERE: the conversion's `call $+5` rewrite; BO2_INLINE_*: accessors inlined, as tools/steam/native_impl.sh
BO2_TARGET=$T BOOT_WORKDIR=boot_cache/$T LIFT_PISTE_ENTIERE=1 BO2_INLINE_FLAGS=1 BO2_INLINE_TRIV=1 BO2_INLINE_READS=1 \
  BO2_INLINE_WRITES=1 python3 -c "
import sys; sys.path.insert(0, 'tools'); import lift_extra
lift_extra.WORKDIR = '$W'
done = lift_extra.lift(lift_extra.read_list('$W/todo.txt'))
open('$W/lifted.txt', 'w').write(''.join(f'{va:x}\n' for va, _ in done))"
VAS=($(cat $W/lifted.txt))
[ ${#VAS} -gt 0 ] || { echo "interp_lift: none of $(wc -l < $W/todo.txt | tr -d ' ') VA lifted"; exit 0; }
for va in $VAS; do $L/clang -arch arm64 -c -O2 -x ir -o $W/$va.o $W/func_$va.bc.fixed; done
# Calls out of these blocks to functions lifted.o lacks: through the dispatcher, as an unknown call target would. Weak,
# so a runtime substitution of the same name still wins at the link (tools/boot.py, SHIM_DEFINED).
$L/llvm-nm -u -j $(for va in $VAS; do echo $W/$va.o; done) | sed -n 's/^_sub_\([0-9a-f]*\)$/\1/p' | sort -u \
  | comm -23 - $W/have.txt | comm -23 - <(printf '%s\n' $VAS | sort) > $W/calls.txt
{ echo '#include <cstdint>'
  echo 'struct State; extern "C" void* __remill_function_call(State&, uint32_t, void*);'
  while read va; do echo "extern \"C\" [[gnu::weak]] void* sub_$va(State* s, uint32_t pc, void* m) { return __remill_function_call(*s, pc, m); }"
  done < $W/calls.txt; } > $W/calls.cpp
# no tail call: each entry keeps the 12 bytes tools/patch_redirect.py writes over a `_sub_<va>`
$L/clang++ -arch arm64 -c -O2 -std=c++17 -fno-optimize-sibling-calls -o $W/calls.o $W/calls.cpp
$L/clang -arch arm64 -r -nostdlib -o $O.new $O $(for va in $VAS; do echo $W/$va.o; done) $W/calls.o
mv $O.new $O
for va in $VAS; do echo "$W/func_$va.bc.fixed"; done >> boot_cache/$T/bc_files.txt
echo "interp_lift: ${#VAS}/$(wc -l < $W/todo.txt | tr -d ' ') VA joined to $O ($(wc -l < $W/calls.txt | tr -d ' ') calls through the dispatcher)"
