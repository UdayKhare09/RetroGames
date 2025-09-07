#!/usr/bin/env bash
set -euo pipefail

# Run the packaged Windows build with Wine
# Usage: ./run-windows.sh [dist_dir]

DIST_DIR="${1:-dist/windows-x86_64}"
EXE="$DIST_DIR/retro_games_collection.exe"

if [ ! -f "$EXE" ]; then
  echo "Executable not found at $EXE" >&2
  exit 1
fi

echo "Running $EXE with Wine"
wine "$EXE"
