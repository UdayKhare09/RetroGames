#!/usr/bin/env bash
set -euo pipefail

# Cross-build Windows x86_64 exe using mingw-w64 and package DLLs into dist/
# Usage: ./build-windows.sh

MINGW_PREFIX="${MINGW_PREFIX:-/usr/x86_64-w64-mingw32}"
TOOLCHAIN="cmake/toolchains/mingw_x86_64.cmake"
BUILD_DIR="build-mingw"
DIST_DIR="dist/windows-x86_64"
PKG_DIR="${PKG_CONFIG_LIBDIR:-$MINGW_PREFIX/lib/pkgconfig}"

echo "MINGW_PREFIX=$MINGW_PREFIX"

# Ensure cross-compiler is available
if ! command -v x86_64-w64-mingw32-gcc >/dev/null 2>&1; then
  echo "x86_64-w64-mingw32-gcc not found in PATH. Install mingw-w64 toolchain or set MINGW_PREFIX." >&2
  exit 1
fi

# Clean stale state and prepare dirs
rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake "$BUILD_DIR"
mkdir -p "$BUILD_DIR" "$DIST_DIR"

# Point pkg-config to the MinGW prefix so SDL2/SDL2_ttf are discovered for cross-builds
export PKG_CONFIG_LIBDIR="$PKG_DIR"
export PKG_CONFIG_PATH="$PKG_DIR"

echo "Configuring with PKG_CONFIG_LIBDIR=$PKG_CONFIG_LIBDIR"
cmake -S . -B "$BUILD_DIR" -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release

echo "Building..."
cmake --build "$BUILD_DIR" --config Release

# Locate built exe
EXE_PATH="$BUILD_DIR/retro_games_collection.exe"
if [ ! -f "$EXE_PATH" ]; then
  # Try case without .exe
  EXE_PATH="$BUILD_DIR/retro_games_collection"
fi

if [ ! -f "$EXE_PATH" ]; then
  echo "Built executable not found in $BUILD_DIR" >&2
  exit 1
fi

# Copy exe to dist
cp -v "$EXE_PATH" "$DIST_DIR/"

# Copy runtime DLLs from MinGW bin to dist
DLLS=(SDL2.dll SDL2_ttf.dll libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll libssp-0.dll libfreetype-6.dll)
for dll in "${DLLS[@]}"; do
  src="$MINGW_PREFIX/bin/$dll"
  if [ -f "$src" ]; then
    cp -v "$src" "$DIST_DIR/" || true
  fi
done

# Also copy any SDL/freetype related DLLs present
shopt -s nullglob || true
for f in "$MINGW_PREFIX/bin/"SDL2*.dll "$MINGW_PREFIX/bin/"freetype*.dll "$MINGW_PREFIX/bin/"libfreetype*.dll; do
  [ -e "$f" ] && cp -vn "$f" "$DIST_DIR/"
done

echo "Windows build and packaging complete. Output: $DIST_DIR"

echo "To run under Wine: ./run-windows.sh $DIST_DIR"
