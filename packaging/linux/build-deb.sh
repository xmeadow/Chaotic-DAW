#!/bin/bash
#
# build-deb.sh — Build a .deb package for Chaotic-DAW
#
# Usage: ./packaging/linux/build-deb.sh [version]
#   e.g.: ./packaging/linux/build-deb.sh 0.9.0
#
set -euo pipefail

VERSION="${1:-0.9.0}"
ARCH="$(dpkg --print-architecture)"
PKG_NAME="chaotic-daw"
PKG_DIR="${PKG_NAME}_${VERSION}_${ARCH}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"

echo "=== Building Chaotic-DAW ${VERSION} .deb package ==="

# --- Build the binary ---
echo "[1/5] Building..."
mkdir -p "$PROJECT_DIR/build"
cd "$PROJECT_DIR/build"
cmake "$PROJECT_DIR" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_CXX_FLAGS_DEBUG="-Og -DNDEBUG"
make -j"$(nproc)"

# Strip debug symbols for smaller package
strip --strip-unneeded Chaotic-DAW

# --- Create package directory structure ---
echo "[2/5] Creating package structure..."
cd "$PROJECT_DIR"
rm -rf "$PKG_DIR"

mkdir -p "$PKG_DIR/DEBIAN"
mkdir -p "$PKG_DIR/usr/bin"
mkdir -p "$PKG_DIR/usr/share/chaotic-daw/Samples"
mkdir -p "$PKG_DIR/usr/share/chaotic-daw/Presets"
mkdir -p "$PKG_DIR/usr/share/chaotic-daw/Projects"
mkdir -p "$PKG_DIR/usr/share/chaotic-daw/Plugins"
mkdir -p "$PKG_DIR/usr/share/applications"
mkdir -p "$PKG_DIR/usr/share/icons/hicolor/256x256/apps"
mkdir -p "$PKG_DIR/usr/share/icons/hicolor/32x32/apps"

# --- Install files ---
echo "[3/5] Installing files..."

# Binary goes to libexec, wrapper script goes to bin
mkdir -p "$PKG_DIR/usr/libexec/chaotic-daw"
cp build/Chaotic-DAW "$PKG_DIR/usr/libexec/chaotic-daw/Chaotic-DAW"

# Symlink assets into libexec so the binary finds them relative to itself
ln -s /usr/share/chaotic-daw/Samples "$PKG_DIR/usr/libexec/chaotic-daw/Samples"
ln -s /usr/share/chaotic-daw/Presets "$PKG_DIR/usr/libexec/chaotic-daw/Presets"
ln -s /usr/share/chaotic-daw/Projects "$PKG_DIR/usr/libexec/chaotic-daw/Projects"
mkdir -p "$PKG_DIR/usr/libexec/chaotic-daw/Plugins"

# Wrapper script
cat > "$PKG_DIR/usr/bin/chaotic-daw" << 'WRAPPER'
#!/bin/bash
exec /usr/libexec/chaotic-daw/Chaotic-DAW "$@"
WRAPPER
chmod +x "$PKG_DIR/usr/bin/chaotic-daw"

# Runtime assets
cp -r Samples/* "$PKG_DIR/usr/share/chaotic-daw/Samples/" 2>/dev/null || true
cp -r Presets/* "$PKG_DIR/usr/share/chaotic-daw/Presets/" 2>/dev/null || true
cp -r Projects/* "$PKG_DIR/usr/share/chaotic-daw/Projects/" 2>/dev/null || true
# Don't include VST plugins in package — users install their own

# Desktop entry & icons
cp packaging/linux/chaotic-daw.desktop "$PKG_DIR/usr/share/applications/"
cp packaging/linux/chaotic-daw.png "$PKG_DIR/usr/share/icons/hicolor/256x256/apps/"
cp packaging/linux/chaotic-daw-32.png "$PKG_DIR/usr/share/icons/hicolor/32x32/apps/chaotic-daw.png"

# --- Compute installed size ---
INSTALLED_SIZE=$(du -sk "$PKG_DIR" | cut -f1)

# --- Create control file ---
echo "[4/5] Writing control file..."
cat > "$PKG_DIR/DEBIAN/control" << EOF
Package: ${PKG_NAME}
Version: ${VERSION}
Section: sound
Priority: optional
Architecture: ${ARCH}
Installed-Size: ${INSTALLED_SIZE}
Depends: libasound2 (>= 1.0), libx11-6, libxext6, libxrandr2, libxinerama1, libxcursor1, libxi6, libsndfile1, libfreetype6, libcurl4 | libcurl4t64, libgl1
Recommends: jackd2
Suggests: vst-plugins
Maintainer: Chaotic-DAW Contributors
Homepage: https://github.com/xmeadow/Chaotic-DAW
Description: Digital Audio Workstation with VST plugin support
 Chaotic-DAW is a music composition and digital audio workstation featuring
 a universal arranger, multiple pattern types (pianoroll, step sequencer),
 VST2 instrument and effect support, built-in synthesizer, mixer, and
 rendering capabilities.
 .
 Features:
  - Piano roll, step sequencer, and text pattern editors
  - VST2 plugin hosting (instruments and effects)
  - Built-in synthesizer and effects (EQ, delay, reverb, etc.)
  - Audio rendering to WAV
  - ALSA and JACK audio output
EOF

# --- Build the .deb ---
echo "[5/5] Building .deb..."
dpkg-deb --build --root-owner-group "$PKG_DIR"

echo ""
echo "=== Done! ==="
echo "Package: ${PKG_DIR}.deb"
ls -lh "${PKG_DIR}.deb"
echo ""
echo "Install with: sudo dpkg -i ${PKG_DIR}.deb"
echo "Remove with:  sudo apt remove ${PKG_NAME}"

# Cleanup build directory
rm -rf "$PKG_DIR"
