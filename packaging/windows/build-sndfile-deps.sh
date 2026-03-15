#!/bin/bash
#
# build-sndfile-deps.sh — Cross-compile libsndfile and dependencies for Windows
#
# Usage: ./packaging/windows/build-sndfile-deps.sh [32|64]
#   e.g.: ./packaging/windows/build-sndfile-deps.sh 64
#
# Builds ogg, flac, vorbis as static libs, then links them into a single
# libsndfile-1.dll. Output goes to packaging/windows/x86/ or x64/.
#
# Prerequisites:
#   32-bit: sudo apt install g++-mingw-w64-i686-win32 cmake git
#   64-bit: sudo apt install g++-mingw-w64-x86-64-win32 cmake git
#
set -euo pipefail

ARCH="${1:-64}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
WORK_DIR="$PROJECT_DIR/_sndfile_build"

if [ "$ARCH" = "64" ]; then
    TRIPLET="x86_64-w64-mingw32"
    OUT_DIR="$SCRIPT_DIR/x64"
elif [ "$ARCH" = "32" ]; then
    TRIPLET="i686-w64-mingw32"
    OUT_DIR="$SCRIPT_DIR/x86"
else
    echo "ERROR: Invalid architecture '$ARCH'. Use 32 or 64."
    exit 1
fi

PREFIX="$WORK_DIR/install_${ARCH}"
TOOLCHAIN="$WORK_DIR/toolchain.cmake"

echo "=== Building libsndfile deps for Windows ${ARCH}-bit ==="
echo "    Triplet:  $TRIPLET"
echo "    Prefix:   $PREFIX"
echo "    Output:   $OUT_DIR"
echo ""

# --- Check prerequisites ---
if ! command -v ${TRIPLET}-g++-win32 &>/dev/null; then
    echo "ERROR: MinGW ${ARCH}-bit cross-compiler not found."
    exit 1
fi

# --- Create work directory ---
mkdir -p "$WORK_DIR"
mkdir -p "$PREFIX"

# --- Write toolchain file ---
cat > "$TOOLCHAIN" << EOF
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER ${TRIPLET}-gcc-win32)
set(CMAKE_CXX_COMPILER ${TRIPLET}-g++-win32)
set(CMAKE_RC_COMPILER ${TRIPLET}-windres)
set(CMAKE_FIND_ROOT_PATH /usr/${TRIPLET} ${PREFIX})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
EOF

# Helper: clone a git repo (skips if already present)
fetch_repo() {
    local name="$1" url="$2" tag="$3"
    local dir="$WORK_DIR/$name"
    if [ -d "$dir" ]; then
        echo "  [skip] $name already cloned"
    else
        echo "  [clone] $name ($tag)..."
        git clone --depth 1 --branch "$tag" "$url" "$dir"
    fi
}

# Helper: build a CMake project (static by default)
build_static() {
    local name="$1"
    shift
    local src="$WORK_DIR/$name"
    local build="$WORK_DIR/${name}_build_${ARCH}"
    echo "  [build] $name (static)..."
    rm -rf "$build"
    mkdir -p "$build"
    cd "$build"
    cmake "$src" \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DCMAKE_PREFIX_PATH="$PREFIX" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DBUILD_TESTING=OFF \
        -DBUILD_PROGRAMS=OFF \
        -DBUILD_EXAMPLES=OFF \
        -DBUILD_DOCS=OFF \
        "$@" \
        > /dev/null 2>&1
    make -j"$(nproc)" > /dev/null 2>&1
    make install > /dev/null 2>&1
    echo "  [done] $name"
}

# --- 1. libogg (static) ---
echo "[1/4] libogg"
fetch_repo ogg https://github.com/xiph/ogg v1.3.5
build_static ogg -DINSTALL_DOCS=OFF

# --- 2. libflac (static) ---
echo "[2/4] libflac"
fetch_repo flac https://github.com/xiph/flac 1.4.3
build_static flac -DWITH_OGG=ON -DBUILD_CXXLIBS=OFF -DINSTALL_MANPAGES=OFF

# --- 3. libvorbis (static) ---
echo "[3/4] libvorbis"
fetch_repo vorbis https://github.com/xiph/vorbis v1.3.7
build_static vorbis

# --- 4. libsndfile (shared — bundles all static deps into one DLL) ---
echo "[4/4] libsndfile"
fetch_repo libsndfile https://github.com/libsndfile/libsndfile 1.2.2

# Patch: libsndfile 1.2.2 unconditionally links Opus even when not found
SNDFILE_SRC="$WORK_DIR/libsndfile"
if grep -q '$<$<BOOL:${HAVE_EXTERNAL_XIPH_LIBS}>:Opus::opus>' "$SNDFILE_SRC/CMakeLists.txt" 2>/dev/null; then
    echo "  [patch] Guarding Opus::opus link target..."
    sed -i 's/\$<\$<BOOL:\${HAVE_EXTERNAL_XIPH_LIBS}>:Opus::opus>/\$<\$<AND:\$<BOOL:\${HAVE_EXTERNAL_XIPH_LIBS}>,\$<BOOL:\${OPUS_FOUND}>>:Opus::opus>/' \
        "$SNDFILE_SRC/CMakeLists.txt"
fi
# Patch: remove ogg_opus.c (needs opus headers we don't have)
if grep -q 'src/ogg_opus\.c' "$SNDFILE_SRC/CMakeLists.txt" 2>/dev/null; then
    echo "  [patch] Removing ogg_opus.c from sources..."
    sed -i '/src\/ogg_opus\.c/d' "$SNDFILE_SRC/CMakeLists.txt"
fi
# Patch: stub out ogg_opus_open call
if grep -q 'return ogg_opus_open' "$SNDFILE_SRC/src/ogg.c" 2>/dev/null; then
    echo "  [patch] Stubbing ogg_opus_open()..."
    sed -i 's/return ogg_opus_open (psf) ;/return SFE_UNIMPLEMENTED ;/' "$SNDFILE_SRC/src/ogg.c"
fi

SNDFILE_BUILD="$WORK_DIR/libsndfile_build_${ARCH}"
echo "  [build] libsndfile (shared)..."
rm -rf "$SNDFILE_BUILD"
mkdir -p "$SNDFILE_BUILD"
cd "$SNDFILE_BUILD"
cmake "$SNDFILE_SRC" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_PREFIX_PATH="$PREFIX" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_TESTING=OFF \
    -DBUILD_PROGRAMS=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DENABLE_EXTERNAL_LIBS=ON \
    -DENABLE_MPEG=OFF \
    -DENABLE_EXPERIMENTAL=OFF \
    -DBUILD_REGTEST=OFF \
    -DCMAKE_DISABLE_FIND_PACKAGE_Opus=TRUE \
    -DENABLE_COMPATIBLE_LIBSNDFILE_NAME=ON \
    -DCMAKE_C_FLAGS="-DFLAC__NO_DLL -DOGG_STATIC" \
    > /dev/null 2>&1
make -j"$(nproc)" > /dev/null 2>&1
make install > /dev/null 2>&1
echo "  [done] libsndfile"

# --- Collect outputs ---
echo ""
echo "Collecting DLLs and import lib..."
mkdir -p "$OUT_DIR"

# DLL
find "$PREFIX/bin" -name "libsndfile*.dll" -exec cp {} "$OUT_DIR/" \;

# Import library
if [ -f "$PREFIX/lib/libsndfile.dll.a" ]; then
    cp "$PREFIX/lib/libsndfile.dll.a" "$OUT_DIR/libsndfile-1-x${ARCH}.lib"
fi

# Strip
${TRIPLET}-strip --strip-unneeded "$OUT_DIR"/*.dll 2>/dev/null || true

echo ""
echo "=== Done! ==="
ls -lh "$OUT_DIR"/
echo ""
echo "To clean up build files: rm -rf $WORK_DIR"
