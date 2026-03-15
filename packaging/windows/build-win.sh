#!/bin/bash
#
# build-win.sh — Cross-compile Chaotic-DAW for Windows and package as ZIP
#
# Usage: ./packaging/windows/build-win.sh [version] [32|64]
#   e.g.: ./packaging/windows/build-win.sh 0.9.0 32   (default, 32-bit)
#         ./packaging/windows/build-win.sh 0.9.0 64   (64-bit)
#
# Prerequisites:
#   32-bit: sudo apt install g++-mingw-w64-i686-win32 zip
#   64-bit: sudo apt install g++-mingw-w64-x86-64-win32 zip
#
set -euo pipefail

VERSION="${1:-0.9.0}"
ARCH="${2:-32}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"

if [ "$ARCH" = "64" ]; then
    TRIPLET="x86_64-w64-mingw32"
    TOOLCHAIN="$SCRIPT_DIR/mingw-toolchain-x86_64.cmake"
    BUILD_DIR="$PROJECT_DIR/build_win64"
    PKG_NAME="Chaotic-DAW_${VERSION}_win64"
    # 64-bit uses seh exceptions, not dwarf
    GCC_DLL="libgcc_s_seh-1.dll"
elif [ "$ARCH" = "32" ]; then
    TRIPLET="i686-w64-mingw32"
    TOOLCHAIN="$SCRIPT_DIR/mingw-toolchain.cmake"
    BUILD_DIR="$PROJECT_DIR/build_win32"
    PKG_NAME="Chaotic-DAW_${VERSION}_win32"
    GCC_DLL="libgcc_s_dw2-1.dll"
else
    echo "ERROR: Invalid architecture '$ARCH'. Use 32 or 64."
    exit 1
fi

PKG_DIR="$PROJECT_DIR/$PKG_NAME"

echo "=== Cross-compiling Chaotic-DAW ${VERSION} for Windows ${ARCH}-bit ==="

# --- Check prerequisites ---
if ! command -v ${TRIPLET}-g++-win32 &>/dev/null; then
    echo "ERROR: MinGW ${ARCH}-bit cross-compiler not found."
    if [ "$ARCH" = "64" ]; then
        echo "Install with: sudo apt install g++-mingw-w64-x86-64-win32"
    else
        echo "Install with: sudo apt install g++-mingw-w64-i686-win32"
    fi
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
${TRIPLET}-strip --strip-unneeded Chaotic-DAW.exe

# --- Create package directory ---
echo "[2/4] Assembling package..."
rm -rf "$PKG_DIR"
mkdir -p "$PKG_DIR"

# Binary
cp "$BUILD_DIR/Chaotic-DAW.exe" "$PKG_DIR/"

# Runtime DLLs
if [ "$ARCH" = "64" ]; then
    DLL_DIR="$SCRIPT_DIR/x64"
else
    DLL_DIR="$SCRIPT_DIR"
fi

if [ "$ARCH" = "64" ]; then
    # 64-bit: single libsndfile DLL (ogg/flac/vorbis statically linked in)
    for dll in libsndfile-1.dll; do
        if [ -f "$DLL_DIR/$dll" ]; then
            cp "$DLL_DIR/$dll" "$PKG_DIR/"
        fi
    done
else
    # 32-bit: separate DLLs
    for dll in \
        libFLAC-0.dll \
        libsndfile-1.dll \
        libvorbis-0.dll \
        libvorbisfile-3.dll \
        zlib1.dll
    do
        if [ -f "$DLL_DIR/$dll" ]; then
            cp "$DLL_DIR/$dll" "$PKG_DIR/"
        fi
    done
fi

# MinGW runtime DLLs (needed for the exe to run)
MINGW_SYSROOT="/usr/lib/gcc/${TRIPLET}"
MINGW_VERSION=$(ls "$MINGW_SYSROOT" 2>/dev/null | sort -V | tail -1)
if [ -n "$MINGW_VERSION" ]; then
    for dll in "$GCC_DLL" libstdc++-6.dll; do
        SRC="$MINGW_SYSROOT/$MINGW_VERSION/$dll"
        if [ -f "$SRC" ]; then
            cp "$SRC" "$PKG_DIR/"
        fi
    done
fi
# Also check the mingw lib directory
for dll in "$GCC_DLL" libstdc++-6.dll libwinpthread-1.dll; do
    SRC="/usr/${TRIPLET}/lib/$dll"
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
