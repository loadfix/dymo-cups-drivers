#!/usr/bin/env bash
# Build a .deb of our patched dymo-cups-drivers that replaces Ubuntu's
# printer-driver-dymo package. The result appears in ../ (one directory
# above this source tree) as printer-driver-dymo_1.4.0-12build3+loadfix1_amd64.deb.
#
# This is a source-form build: dpkg-buildpackage runs autoreconf + configure +
# make + makes a binary-only .deb. It applies debian/patches/* on top of our
# working tree via quilt, then `make install DESTDIR=debian/tmp`, then assembles
# .deb.
#
# Requirements:
#   build-essential autoconf automake libtool quilt debhelper
#   cups-client libcppunit-dev libcups2-dev libcupsimage2-dev pyppd
#
# After a successful build, install with:
#   sudo dpkg -i ../printer-driver-dymo_1.4.0-12build3+loadfix1_amd64.deb
set -euo pipefail

cd "$(dirname "$0")"

if ! command -v dpkg-buildpackage >/dev/null; then
  echo "dpkg-buildpackage not installed. Install build-essential debhelper." >&2
  exit 1
fi

# Install build deps the same way Ubuntu would
missing=$(dpkg-checkbuilddeps 2>&1 | sed -n 's/.*Unmet build deps: //p' || true)
if [ -n "$missing" ]; then
  echo "Missing build deps: $missing"
  echo "Installing via apt..."
  sudo apt install -y $(echo "$missing" | tr -d '()><=' | sed 's/[0-9.-]\+//g')
fi

# Binary-only, unsigned build
dpkg-buildpackage -b -us -uc

echo
echo "Done. Artifact:"
ls -1 ../printer-driver-dymo_*_amd64.deb 2>/dev/null || {
  echo "  (no .deb found in ../ — something went wrong)" >&2
  exit 1
}
