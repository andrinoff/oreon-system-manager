#!/usr/bin/env bash
set -euo pipefail

cargo build --release

# Install icon (multiple sizes for desktop integration)
INSTALL_PREFIX="${1:-/usr}"

# Install 200x200 PNG icon
install -Dm644 assets/logo.png \
    "${DESTDIR:-}${INSTALL_PREFIX}/share/icons/hicolor/200x200/apps/oreon-system-manager.png"

# Install desktop file
install -Dm644 packaging/oreon-system-manager.desktop \
    "${DESTDIR:-}${INSTALL_PREFIX}/share/applications/oreon-system-manager.desktop"

# Install binary
install -Dm755 target/release/oreon-system-manager \
    "${DESTDIR:-}${INSTALL_PREFIX}/bin/oreon-system-manager"

echo "Installed to ${INSTALL_PREFIX}"
echo "Run 'gtk-update-icon-cache' to refresh the icon cache"
