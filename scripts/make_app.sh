#!/bin/bash
# make_app.sh [folder] — builds "Black Ops II.app" (default: /Applications): double-click = the campaign at the main
# menu, no panel, the way Steam launches t6sp; ZOMBIES / MULTIPLAYER are launched from the game's own menus
# (bo2_relais_jeu relay -> scripts/launch.sh). The icon is read from the user's t6sp.exe, never stored here.
# No Metal HUD and no counters in the title (MTL_HUD_ENABLED=0, BO2_TITRE=0): the test launchers keep them.
# Launcher log: ~/Library/Logs/BlackOpsII.log.
#
# The bundle carries the sources only (Contents/Resources/bo2), no byte taken from the game. On first launch it asks
# for the game folder, then install_steam.sh and build_steam.sh translate the player's own Steam executables into
# ~/.bo2-native/app-local, behind the preparation window (tools/setup_window.swift). The bundle stays read-only, so its
# signature holds. `--from-source` is accepted for compatibility (it is the only mode).
# No Terminal for the player: when boot_cache/toolchain exists (scripts/make_toolchain.sh; BO2_TOOLCHAIN_SRC: another
# folder), the bundle carries it (LLVM 17, Remill, libvpx, Unicorn, HLSLcc, capstone) and the player only needs Apple's
# Command Line Tools, which the preparation window asks macOS to install.
# Release: BO2_SIGN="Developer ID Application" signs every binary with the hardened runtime (team 2TWQF4T93E), and
# BO2_NOTARY=<notarytool keychain profile> then has Apple notarize the bundle and staples the ticket.
set -eu
ICI="$(cd "$(dirname "$0")/.." && pwd)"
[ "${1:-}" = --from-source ] && shift
DEST="${1:-/Applications}"
APP="$DEST/${BO2_APP_NAME:-Black Ops II}.app"   # BO2_APP_NAME: another bundle name
TC="${BO2_TOOLCHAIN_SRC:-$ICI/boot_cache/toolchain}"
MINOS=14.0; [ -f "$TC/minos" ] && MINOS=$(cat "$TC/minos")   # the toolchain's binaries set the oldest macOS
rm -rf "$APP"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources"
cat > "$APP/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
  <key>CFBundleName</key><string>Black Ops II</string>
  <key>CFBundleDisplayName</key><string>Black Ops II</string>
  <key>CFBundleIdentifier</key><string>local.bo2-native.launcher</string>
  <key>CFBundleExecutable</key><string>BlackOpsII</string>
  <key>CFBundleIconFile</key><string>AppIcon</string>
  <key>CFBundlePackageType</key><string>APPL</string>
  <key>CFBundleShortVersionString</key><string>2.0</string>
  <key>LSMinimumSystemVersion</key><string>$MINOS</string>
</dict></plist>
EOF

# The mini-repository: same relative paths as in the repository, so the launchers and the relay need not know.
B="$APP/Contents/Resources/bo2"
mkdir -p "$B"
# Tracked files only (with their uncommitted edits): nothing untracked of this checkout leaves the Mac.
(cd "$ICI" && git ls-files -z | tar --null -T - -cf -) | tar -xf - -C "$B"   # macOS rsync has no --from0
[ ! -d "$TC" ] || ditto "$TC" "$APP/Contents/Resources/toolchain"
swiftc -O -target arm64-apple-macos$MINOS -o "$APP/Contents/MacOS/preparation" "$ICI/tools/setup_window.swift"
I="$(mktemp -d)/AppIcon.iconset"; mkdir -p "$I"   # our neutral mark until the player's t6sp.exe gives the game's icon
"$APP/Contents/MacOS/preparation" --icone "$I/icon_512x512@2x.png"
for s in 16 32 128 256 512; do sips -z $s $s "$I/icon_512x512@2x.png" --out "$I/icon_${s}x${s}.png" >/dev/null
  [ $s = 512 ] || sips -z $((s * 2)) $((s * 2)) "$I/icon_512x512@2x.png" --out "$I/icon_${s}x${s}@2x.png" >/dev/null; done
iconutil -c icns "$I" -o "$APP/Contents/Resources/AppIcon.icns"
# The bundle's executable is a Mach-O that runs Resources/launcher.sh: the notary service signs and checks Mach-O.
printf '%s\n' '#include <libgen.h>' '#include <mach-o/dyld.h>' '#include <stdio.h>' '#include <unistd.h>' \
  'int main(void) { char p[4096], s[4200]; uint32_t n = sizeof p; if (_NSGetExecutablePath(p, &n)) return 1;' \
  '  snprintf(s, sizeof s, "%s/../Resources/launcher.sh", dirname(p)); execl("/bin/bash", "bash", s, (char *)0); return 1; }' \
  | clang -O2 -mmacosx-version-min=$MINOS -x c -o "$APP/Contents/MacOS/BlackOpsII" -
touch "$APP/Contents/Resources/from-source"
echo "$(date '+%Y-%m-%d %H:%M:%S') $(git -C "$ICI" rev-parse --short HEAD)" > "$APP/Contents/Resources/version"

cat > "$APP/Contents/Resources/launcher.sh" <<'EOF'
#!/bin/bash
# Clones Resources/bo2 into ~/.bo2-native/app-local when the version changes (the working copy, where the game writes), then launches.
R="$(cd "$(dirname "$0")" && pwd)"
W="$HOME/.bo2-native/app-local"
exec >> "$HOME/Library/Logs/BlackOpsII.log" 2>&1
export PATH="$PATH:/opt/homebrew/bin"   # launched by Finder: no Homebrew cmake/ninja; appended, so python3 stays the one that passed the capstone check
echo "--- $(date +%H:%M:%S) bundle $(cat "$R/version")"
case "$(defaults read -g AppleLanguages 2>/dev/null | tr -dc 'a-zA-Z-\n' | sed -n 2p | cut -c1-2)" in   # copy failed
  fr) T3="Copie impossible vers";;
  de) T3="Kopieren nicht möglich nach";;
  es) T3="No se puede copiar a";;
  it) T3="Impossibile copiare in";;
  pt) T3="Não foi possível copiar para";;
  pl) T3="Nie można skopiować do";;
  ru) T3="Не удалось скопировать в";;
  ja) T3="コピーできません:";;
  *)  T3="Cannot copy to";;
esac
alerte() { osascript -e "display alert \"Black Ops II\" message \"$1\"" >/dev/null; exit 1; }
G="$HOME/.bo2-native/game_dir"; [ -s "$G" ] || [ ! -s "$HOME/.bo2-native/jeu" ] || G="$HOME/.bo2-native/jeu"   # older name
if ! cmp -s "$R/version" "$W/.version"; then
  echo "working copy -> $W"
  mkdir -p "$W" && ditto --clone "$R/bo2" "$W" && cp "$R/version" "$W/.version" || alerte "$T3 $W"
fi
# The bundle's tools, read in place (it may move, or be translocated) through a link with no space in its path: the
# build scripts take tool paths unquoted, and the bundle is "Black Ops II.app".
[ ! -d "$R/toolchain" ] || { ln -sfn "$R/toolchain" "$W/.toolchain" && export BO2_TOOLCHAIN="$W/.toolchain"; }
mkdir -p "$W/boot_cache" && echo "${R%/Contents/Resources}" > "$W/boot_cache/app_path"   # reopened by play_steam.sh
C=$(cat "$W/boot_cache/conversion_target" 2>/dev/null); case "$C" in sps|zms) ;; *) C=mps ;; esac   # the modes asked for (issue #3)
# Welcome window: the game folder and the three prepared exes, before anything runs (tools/setup_window.swift --accueil).
[ ! -s "$G" ] || bash "$W/scripts/game_icon.sh" "$(cat "$G")" || true   # the game's icon in the window, when the folder is known
X=0; for e in t6sp t6mp t6zm; do [ -s "$HOME/.bo2-native/steam/$e.exe.unpacked.exe" ] || X=1; done
if [ ! -s "$G" ] || { [ $X = 1 ] && [ ! -x "$W/boot_cache/$C/boot" ]; }; then
  J=$("$R/../MacOS/preparation" --accueil) && [ -n "$J" ] || exit 0   # window closed
  mkdir -p "$HOME/.bo2-native" && printf '%s\n' "$J" > "$G"
fi
if [ -f "$R/from-source" ] && [ ! -x "$W/boot_cache/$C/boot" ]; then   # first launch: translate the player's t6sp.exe
  bash "$W/scripts/game_icon.sh" "$(cat "$G")" || true
  while :; do
    P=$(cat "$W/preparation.pid" 2>/dev/null) || true
    if [ -z "$P" ] || ! kill -0 "$P" 2>/dev/null; then            # a preparation already running: just show it
      (cd "$W" && mkdir -p boot_cache && bash scripts/install_steam.sh "$(cat "$G")" \
         && bash scripts/build_steam.sh) > "$W/preparation.log" 2>&1 &
      P=$!; echo $P > "$W/preparation.pid"
      caffeinate -i -w "$P" &                                     # no sleep while the preparation runs
    fi
    "$R/../MacOS/preparation" "$W/preparation.log" "$P"; rc=$?
    [ $rc = 3 ] || break                                          # 3 = "Try Again" (build_steam.sh resumes)
  done
  if [ $rc = 4 ]; then                                            # 4 = play the campaign now: stop the whole conversion tree
    echo sps > "$W/boot_cache/conversion_target"                  # and keep the other modes for when they are picked
    tue() { kill -STOP "$1" 2>/dev/null; for c in $(pgrep -P "$1"); do tue "$c"; done; kill -KILL "$1" 2>/dev/null; }
    trap '' HUP; tue "$P"; trap - HUP   # launchd's child = orphaned group: a death beside stopped members HUPs it all, us too
    cd "$W" && exec env BO2_PANNEAU=0 MTL_HUD_ENABLED=0 BO2_TITRE=0 scripts/launch.sh campagne
  fi
  [ $rc = 0 ] && [ -x "$W/boot_cache/$C/boot" ] || exit 0           # window closed: the preparation goes on without it
fi
cd "$W" && exec env BO2_PANNEAU=0 MTL_HUD_ENABLED=0 BO2_TITRE=0 scripts/launch.sh campagne
EOF
chmod +x "$APP/Contents/Resources/launcher.sh"

if [ -n "${BO2_SIGN:-}" ]; then   # inside out: every Mach-O, then the bundle, which seals the rest
  find "$APP/Contents" -type f | while read -r f; do
    if file -b "$f" | grep -q Mach-O; then
      codesign -s "$BO2_SIGN" -f -o runtime --timestamp "$f" 2>/dev/null || { echo "codesign failed: $f"; exit 1; }
    fi
  done
  codesign -s "$BO2_SIGN" -f -o runtime --timestamp "$APP"
  codesign --verify --deep --strict "$APP"
  codesign -dv "$APP" 2>&1 | grep -q '^TeamIdentifier=2TWQF4T93E$' || { echo "not signed by team 2TWQF4T93E"; exit 1; }
else
  codesign -s - --force "$APP" 2>/dev/null || true   # ad-hoc signature: Gatekeeper does not open an unsigned bundle
fi
if [ -n "${BO2_NOTARY:-}" ]; then   # Apple's check of the zip, then the ticket stapled so Gatekeeper needs no network
  Z="$(mktemp -d)/bo2-mac.zip"; ditto -c -k --keepParent "$APP" "$Z"
  xcrun notarytool submit "$Z" --keychain-profile "$BO2_NOTARY" --wait | tee /dev/stderr | grep -q 'status: Accepted'
  xcrun stapler staple "$APP"; rm -rf "$(dirname "$Z")"
fi
touch "$APP"
echo "$APP"
