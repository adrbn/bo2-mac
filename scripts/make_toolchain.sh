#!/bin/bash
# make_toolchain.sh [out=boot_cache/toolchain] — the translation tools the app carries (make_app.sh), so a player needs
# neither Homebrew nor Terminal: Apple's Command Line Tools only (SDK, ld, python3), which macOS offers in a window.
# Built here from Homebrew (llvm@17, libvpx, unicorn, zstd), patched Remill v6.0.1, HLSLcc and the capstone wheel.
# NO game byte: nothing here reads ~/.bo2-native. Laid out like Homebrew's opt/ (+ remill/, hlslcc/, python/), so
# BO2_TOOLCHAIN=<out> stands in for /opt/homebrew/opt (tools/boot.py, scripts/build_steam.sh).
# Remill and HLSLcc are built under /tmp/bo2-mac-*: their build folder is written into the binaries, and must not be
# a home folder. The run fails if a binary still names /Users/ or /opt/homebrew.
set -eu
cd "$(dirname "$0")/.."
ROOT=$PWD; B=/opt/homebrew/opt; L=$B/llvm@17
OUT=${1:-boot_cache/toolchain}; mkdir -p "$(dirname "$OUT")"; OUT="$(cd "$(dirname "$OUT")" && pwd)/$(basename "$OUT")"
RS=/tmp/bo2-mac-remill; HS=/tmp/bo2-mac-hlslcc
CAPSTONE=capstone-5.0.7-py3-none-macosx_11_0_arm64.whl
CAPSTONE_SHA=acb89f5bf6f625745a104a3a44819d3acea173228055c1eadc60d2282ae490bb
for d in llvm@17 libvpx unicorn zstd; do [ -d $B/$d ] || { echo "missing: brew install $d"; exit 1; }; done
minos() { otool -l "$1" 2>/dev/null | awk '/minos/ {print $2; exit}'; }
# The oldest macOS the app runs on = the Homebrew bottles' (Homebrew only bottles the current macOS): Remill and HLSLcc
# are built for it too, not for this Mac's macOS. make_app.sh reads it back for LSMinimumSystemVersion.
export MACOSX_DEPLOYMENT_TARGET=$(minos $L/bin/clang-17)

# Remill (patched, tools/remill-patches) and HLSLcc, both outside the home folder.
mkdir -p boot_cache
[ "$(minos $RS/build/bin/lift/remill-lift-17)" = $MACOSX_DEPLOYMENT_TARGET ] || rm -rf $RS/build $RS/build-deps $RS/deps-install
[ "$(minos $HS/libhlslcc.dylib)" = $MACOSX_DEPLOYMENT_TARGET ] || rm -rf $HS
REMILL_SRC=$RS bash scripts/build_remill.sh
[ -s $HS/libhlslcc.dylib ] || {
  cmake -S third_party/HLSLcc -B $HS -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 > /dev/null
  cmake --build $HS -j"${J:-2}" --target hlslcc > /dev/null; }

rm -rf "$OUT"; mkdir -p "$OUT"; echo $MACOSX_DEPLOYMENT_TARGET > "$OUT/minos"
T="$OUT/llvm@17"; mkdir -p "$T/bin" "$T/lib/clang/17/lib/darwin" "$T/include" "$OUT/licenses"
# LLVM 17: the tools the build calls, clang's own headers, libc++'s headers (the dylib is the system's).
cp $L/bin/{clang-17,opt,llvm-dis,llvm-as,llvm-link,llvm-nm,llvm-objcopy} "$T/bin/"
ln -s clang-17 "$T/bin/clang"; ln -s clang "$T/bin/clang++"
cp -R $L/lib/clang/17/include "$T/lib/clang/17/"
cp $L/lib/clang/17/lib/darwin/libclang_rt.osx.a "$T/lib/clang/17/lib/darwin/"
cp -R $L/include/c++ "$T/include/"
for d in libvpx unicorn; do mkdir -p "$OUT/$d/lib"; cp -R $B/$d/include "$OUT/$d/"; done
cp $B/libvpx/lib/libvpx.a "$OUT/libvpx/lib/"; cp $B/unicorn/lib/libunicorn.a "$OUT/unicorn/lib/"
R="$OUT/remill/build"; mkdir -p "$R/bin/lift" "$R/lib/Arch/X86/Runtime"
cp $RS/build/bin/lift/remill-lift-17 "$R/bin/lift/"; cp $RS/build/lib/Arch/X86/Runtime/x86.bc "$R/lib/Arch/X86/Runtime/"
cp -R $RS/include "$OUT/remill/"
mkdir -p "$OUT/hlslcc"; cp $HS/libhlslcc.dylib "$OUT/hlslcc/"
install_name_tool -id @rpath/libhlslcc.dylib "$OUT/hlslcc/libhlslcc.dylib"   # found through DYLD_LIBRARY_PATH (play_steam.sh)
W=$(mktemp -d); pip3 download -q capstone==5.0.7 --only-binary :all: --platform macosx_11_0_arm64 --no-deps -d "$W"
[ "$(shasum -a 256 "$W/$CAPSTONE" | cut -c1-64)" = $CAPSTONE_SHA ] || { echo "capstone wheel: wrong SHA-256"; exit 1; }
mkdir -p "$OUT/python"; (cd "$OUT/python" && unzip -q "$W/$CAPSTONE" 'capstone/*' && rm -rf capstone/include)
unzip -p "$W/$CAPSTONE" '*/LICENSE*' > "$OUT/licenses/capstone.txt" 2>/dev/null || true; rm -rf "$W"
chmod -R u+w "$OUT"   # Homebrew's files are read-only: install_name_tool and codesign rewrite them

# Every /opt/homebrew library a binary loads moves to llvm@17/lib and is loaded through @rpath (twice: libLLVM's own).
macho() { find "$OUT" -type f \( -perm +111 -o -name '*.dylib' \) -exec sh -c 'file -b "$1" | grep -q Mach-O' _ {} \; -print; }
for pass in 1 2; do
  for f in $(macho); do
    for dep in $(otool -L "$f" | awk 'NR>1 && $1 ~ /^\/opt\/homebrew\// {print $1}'); do
      n=$(basename "$dep"); [ -f "$T/lib/$n" ] || { cp "$dep" "$T/lib/$n"; chmod u+w "$T/lib/$n"; }
      [ "$(otool -D "$f" | sed -n 2p)" = "$dep" ] && install_name_tool -id "@rpath/$n" "$f" 2>/dev/null \
        || install_name_tool -change "$dep" "@rpath/$n" "$f" 2>/dev/null
    done
  done
done
for lib in libLLVM.dylib libclang-cpp.dylib; do [ -f "$T/lib/$lib" ] || cp $L/lib/$lib "$T/lib/"; done
for f in "$T"/lib/*.dylib; do chmod u+w "$f"; install_name_tool -id "@rpath/$(basename "$f")" "$f" 2>/dev/null; done
install_name_tool -add_rpath @loader_path/../../../../llvm@17/lib "$R/bin/lift/remill-lift-17" 2>/dev/null || true
for f in $(macho); do codesign -s - -f "$f" 2>/dev/null; done   # install_name_tool voids signatures (make_app.sh re-signs)

cp $L/LICENSE.TXT "$OUT/licenses/LLVM.txt"; cp $B/libvpx/LICENSE "$OUT/licenses/libvpx.txt"
# Unicorn's source, for the release next to the app (GPL-2.0 section 3: the object code ships with its source).
UV=$(basename "$(readlink $B/unicorn)")   # ../Cellar/unicorn/<version>
brew fetch -q -s unicorn >/dev/null && cp "$(brew --cache -s unicorn)" "$(dirname "$OUT")/unicorn-$UV-source.tar.gz"
cp $B/unicorn/COPYING "$OUT/licenses/unicorn-GPL-2.0.txt"; cp $B/zstd/LICENSE "$OUT/licenses/zstd.txt" 2>/dev/null || true
cp $RS/LICENSE "$OUT/licenses/remill.txt"; cp third_party/HLSLcc/license.txt "$OUT/licenses/HLSLcc.txt"
cat > "$OUT/licenses/README.txt" <<EOF
Third-party tools carried by Black Ops II.app, each under its own license (files next to this one):
LLVM 17.0.6 (Apache-2.0 WITH LLVM-exception), Remill v6.0.1 + tools/remill-patches (Apache-2.0; statically includes
Intel XED, Apache-2.0, and glog/gflags, BSD-3-Clause), HLSLcc (MIT), libvpx (BSD-3-Clause), zstd (BSD-3-Clause),
capstone 5.0.7 (BSD-3-Clause), Unicorn $UV (GPL-2.0, library only: linked on the player's Mac into a binary that is
never distributed; its source, unicorn-$UV-source.tar.gz, is published with each release). Sources:
https://github.com/llvm/llvm-project/tree/llvmorg-17.0.6,
https://github.com/lifting-bits/remill/tree/v6.0.1, https://github.com/webmproject/libvpx,
https://github.com/facebook/zstd, https://github.com/capstone-engine/capstone/tree/5.0.7,
https://github.com/unicorn-engine/unicorn/tree/$UV.
EOF

# Gates: nothing left pointing at this Mac's Homebrew or home folder, and the tools start.
BAD=$(for f in $(macho); do otool -L "$f" | awk -v f="$f" 'NR>1 && $1 ~ /^\/(opt\/homebrew|Users)\// {print f": "$1}'; done)
[ -z "$BAD" ] || { echo "still loads from Homebrew: $BAD"; exit 1; }
grep -rl -a "/Users/" "$OUT" && { echo "a file above names a home folder"; exit 1; }
NEW=$(for f in $(macho); do v=$(minos "$f"); [ -z "$v" ] || [ "$(printf '%s\n' $v $MACOSX_DEPLOYMENT_TARGET | sort -V | tail -1)" = $MACOSX_DEPLOYMENT_TARGET ] || echo "$f: $v"; done)
[ -z "$NEW" ] || { echo "built for a newer macOS than $MACOSX_DEPLOYMENT_TARGET: $NEW"; exit 1; }
"$T/bin/clang" --version | head -1 && "$T/bin/opt" --version | grep -m1 version
"$R/bin/lift/remill-lift-17" --help 2>&1 | grep -q -- -serve || { echo "remill-lift does not start"; exit 1; }
echo "$OUT: $(du -sh "$OUT" | cut -f1)  (release asset: $(dirname "$OUT")/unicorn-$UV-source.tar.gz)"
