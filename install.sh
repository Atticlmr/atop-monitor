#!/bin/bash
# One-line install script for atop-monitor
# Usage: curl -sSL https://raw.githubusercontent.com/Atticlmr/atop-monitor/main/install.sh | bash

set -e

REPO="${REPO:-Atticlmr/atop-monitor}"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"
VERSION="${VERSION:-latest}"
TMPDIR=$(mktemp -d)
ARCH=$(uname -m)
OS=$(uname -s)

echo "Installing atop-monitor from $REPO..."

if [ "$OS" != "Linux" ]; then
    echo "Error: Only Linux is supported"
    exit 1
fi

case "$ARCH" in
    x86_64) ARCH_NAME="amd64" ;;
    aarch64) ARCH_NAME="arm64" ;;
    armv7l) ARCH_NAME="armhf" ;;
    *)
        echo "Error: Unsupported architecture: $ARCH"
        exit 1
        ;;
esac

DOWNLOAD_URL="https://github.com/$REPO/releases/download/${VERSION}/atop-monitor-linux-${ARCH_NAME}"

if command -v wget >/dev/null 2>&1; then
    wget -q -O "$TMPDIR/atop-monitor" "$DOWNLOAD_URL" || {
        echo "Error: Failed to download from $DOWNLOAD_URL"
        echo "Please ensure the release asset exists at: $DOWNLOAD_URL"
        exit 1
    }
else
    curl -sSL -o "$TMPDIR/atop-monitor" "$DOWNLOAD_URL" || {
        echo "Error: Failed to download from $DOWNLOAD_URL"
        echo "Please ensure the release asset exists at: $DOWNLOAD_URL"
        exit 1
    }
fi

chmod +x "$TMPDIR/atop-monitor"

if [ -w "$INSTALL_PREFIX/bin" ]; then
    cp "$TMPDIR/atop-monitor" "$INSTALL_PREFIX/bin/atop-monitor"
else
    echo "Installing to $INSTALL_PREFIX/bin (requires sudo)..."
    sudo cp "$TMPDIR/atop-monitor" "$INSTALL_PREFIX/bin/atop-monitor"
fi

rm -rf "$TMPDIR"

echo ""
echo "✅ atop-monitor installed successfully!"
echo "   Location: $INSTALL_PREFIX/bin/atop-monitor"
echo ""
echo "Run 'atop-monitor' to start."
