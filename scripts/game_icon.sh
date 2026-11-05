#!/bin/bash
# game_icon.sh <game folder> — the game's icon, read from the player's t6sp.exe, as icon.png at the root (once). Used by
# the launcher and the welcome window (tools/setup_window.swift), which show the app's own icon until it exists.
cd "$(dirname "$0")/.." || exit 1
[ -s icon.png ] || { python3 tools/exe_icon.py "$1/t6sp.exe" icon.ico && sips -s format png icon.ico --out icon-tmp.png >/dev/null \
  && bash scripts/icon_margin.sh icon-tmp.png && mv icon-tmp.png icon.png; }   # renamed when done: the window may be reading it
