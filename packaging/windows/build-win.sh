#!/bin/bash
#
# build-win.sh — Cross-compile Chaotic-DAW for Windows and package as ZIP
#
# Usage: ./packaging/windows/build-win.sh [version]
#   e.g.: ./packaging/windows/build-win.sh 0.9.0
#
# Prerequisites:
#   sudo apt install g++-mingw-w64-i686-win32 zip
#
set -euo pipefail

VERSION="${1:-0.9.0}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build_win32"
PKG_NAME="Chaotic-DAW_${VERSION}_win32"
PKG_DIR="$PROJECT_DIR/$PKG_NAME"
TOOLCHAIN="$SCRIPT_DIR/mingw-toolchain.cmake"

echo "=== Cross-compiling Chaotic-DAW ${VERSION} for Windows ==="

# --- Check prerequisites ---
if ! command -v i686-w64-mingw32-g++-win32 &>/dev/null; then
    echo "ERROR: MinGW cross-compiler not found."
    echo "Install with: sudo apt install g++-mingw-w64-i686-win32"
    exit 1
fi
if ! command -v zip &>/dev/null; then
    echo "ERROR: zip not found."
    echo "Install with: sudo apt install zip"
    exit 1
fi

# --- Build ---
echo "[1/4] Building..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Copy RC resources into build dir (windres looks for them relative to cwd)
cp "$SCRIPT_DIR/Awful.ico" "$SCRIPT_DIR/Small.ico" "$SCRIPT_DIR/cursor1.cur" "$BUILD_DIR/"

cmake "$PROJECT_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DCMAKE_BUILD_TYPE=Release
make -j"$(nproc)"

# Strip debug symbols
i686-w64-mingw32-strip --strip-unneeded Chaotic-DAW.exe

# --- Create package directory ---
echo "[2/4] Assembling package..."
rm -rf "$PKG_DIR"
mkdir -p "$PKG_DIR"

# Binary
cp "$BUILD_DIR/Chaotic-DAW.exe" "$PKG_DIR/"

# Runtime DLLs from packaging/windows
for dll in \
    libFLAC-0.dll \
    libsndfile-1.dll \
    libvorbis-0.dll \
    libvorbisfile-3.dll \
    zlib1.dll
do
    if [ -f "$SCRIPT_DIR/$dll" ]; then
        cp "$SCRIPT_DIR/$dll" "$PKG_DIR/"
    fi
done

# MinGW runtime DLLs (needed for the exe to run)
MINGW_SYSROOT="/usr/lib/gcc/i686-w64-mingw32"
MINGW_VERSION=$(ls "$MINGW_SYSROOT" 2>/dev/null | sort -V | tail -1)
if [ -n "$MINGW_VERSION" ]; then
    for dll in libgcc_s_dw2-1.dll libstdc++-6.dll; do
        SRC="$MINGW_SYSROOT/$MINGW_VERSION/$dll"
        if [ -f "$SRC" ]; then
            cp "$SRC" "$PKG_DIR/"
        fi
    done
fi
# Also check the mingw lib directory
for dll in libgcc_s_dw2-1.dll libstdc++-6.dll libwinpthread-1.dll; do
    SRC="/usr/i686-w64-mingw32/lib/$dll"
    if [ -f "$SRC" ] && [ ! -f "$PKG_DIR/$dll" ]; then
        cp "$SRC" "$PKG_DIR/"
    fi
done

# Icons
cp "$SCRIPT_DIR/Awful.ico" "$PKG_DIR/" 2>/dev/null || true
cp "$SCRIPT_DIR/Small.ico" "$PKG_DIR/" 2>/dev/null || true

# Runtime assets
for dir in Samples Presets Projects Plugins; do
    if [ -d "$PROJECT_DIR/$dir" ]; then
        cp -r "$PROJECT_DIR/$dir" "$PKG_DIR/"
    fi
done

# Remove Linux .so plugins from Windows package
find "$PKG_DIR/Plugins" -name '*.so' -delete 2>/dev/null || true

# --- Create ZIP ---
echo "[3/4] Creating ZIP..."
cd "$PROJECT_DIR"
rm -f "${PKG_NAME}.zip"
zip -r "${PKG_NAME}.zip" "$PKG_NAME"

# --- Done ---
echo "[4/4] Cleanup..."
rm -rf "$PKG_DIR"

echo ""
echo "=== Done! ==="
echo "Package: ${PKG_NAME}.zip"
ls -lh "${PKG_NAME}.zip"
echo ""
echo "Extract on Windows and run Chaotic-DAW.exe"
