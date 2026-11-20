#!/bin/zsh
# send_back.sh sps|mps|zms <list> : sends translated functions back to the interpreter, to find a wrong one by halving.
# <list>: `va[:size]` lines (hex, # comments), the tools/lift_extra.py --list format. Every translated entry inside one
# of them (boot_cache/<mode>/bc_files.txt) gets a jump to bo2_send_back written over its first instructions
# (tools/patch_redirect.py), then the game runs with BO2_INTERP_FORCE=<list> (runtime/win32/win32_runtime.cpp), so
# neither a direct call nor a table dispatch reaches the translation: the interpreter runs its original bytes.
# The translated boot is kept as boot.translated; `send_back.sh <mode> off` puts it back.
# Halving: send half the suspects back; the bug gone means it is in that half, still there means the other half.
set -e -o pipefail
cd "$(dirname $0)/../.."
T=$1; D=boot_cache/$T; B=$D/boot; K=$D/send_back.txt
[[ $T == (sps|mps|zms) ]] && [ -n "${2:-}" ] || { echo "usage: send_back.sh sps|mps|zms <list>|off"; exit 1 }
pgrep -x boot >/dev/null && { echo "send_back: a game is running, close it first"; exit 1 }
# a boot newer than the last send-back is a new link: it becomes the translated one
{ [ -f $K ] && [ ! $B -nt $K ]; } || cp -p $B $B.translated
cp -p $B.translated $B
if [ $2 = off ]; then rm -f $K; echo "send_back: $B is the translated one again"; exit 0; fi
[ -f $2 ] || { echo "send_back: no file $2"; exit 1 }
nm $B | grep -q ' _bo2_send_back$' || { echo "send_back: $B predates bo2_send_back, convert (relink) again first"; exit 1 }
P=($(python3 - $2 $D/bc_files.txt <<'EOF'
import re, sys
r = []
for l in open(sys.argv[1]):
    m = re.match(r"\s*(?:0x)?([0-9a-fA-F]+)(?::(?:0x)?([0-9a-fA-F]+))?", l.split("#")[0])
    if m: r.append((int(m[1], 16), int(m[1], 16) + (int(m[2], 16) if m[2] else 1)))
vas = {int(v, 16) for v in re.findall(r"func_([0-9a-f]+)\.bc", open(sys.argv[2]).read())}
print(" ".join(f"_sub_{v:x}=_bo2_send_back" for v in sorted(vas) if any(lo <= v < hi for lo, hi in r)))
EOF
))
[ ${#P} -gt 0 ] || { echo "send_back: no translated entry inside $2"; exit 1 }
python3 tools/patch_redirect.py $B $P | tail -1
codesign -f -s - $B 2>/dev/null
cp $2 $K
echo "send_back: ${#P} translated entries sent back. Play with: scripts/play_steam.sh ${T:0:2} BO2_INTERP_FORCE=$PWD/$K"
