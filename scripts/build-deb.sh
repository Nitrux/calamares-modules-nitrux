#!/usr/bin/env bash

# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024-2025 <Nitrux Latinoamericana S.C. <hello@nxos.org>>


# -- Exit on errors.

set -euo pipefail


# -- Compile Source

rm -rf build
mkdir -p build && cd build

HOST_MULTIARCH=$(dpkg-architecture -qDEB_HOST_MULTIARCH)
NITRUX_CPPFLAGS=$(dpkg-buildflags --get CPPFLAGS)
NITRUX_CFLAGS=$(dpkg-buildflags --get CFLAGS)
NITRUX_CXXFLAGS=$(dpkg-buildflags --get CXXFLAGS)
NITRUX_LDFLAGS=$(dpkg-buildflags --get LDFLAGS)

cmake \
	-DCMAKE_INSTALL_PREFIX=/usr \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_FLAGS="${NITRUX_CPPFLAGS} ${NITRUX_CFLAGS}" \
	-DCMAKE_CXX_FLAGS="${NITRUX_CPPFLAGS} ${NITRUX_CXXFLAGS}" \
	-DCMAKE_EXE_LINKER_FLAGS="${NITRUX_LDFLAGS}" \
	-DCMAKE_SHARED_LINKER_FLAGS="${NITRUX_LDFLAGS}" \
	-DCMAKE_INSTALL_SYSCONFDIR=/etc \
	-DCMAKE_INSTALL_LOCALSTATEDIR=/var \
	-DCMAKE_EXPORT_NO_PACKAGE_REGISTRY=ON \
	-DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON \
	-DCMAKE_INSTALL_RUNSTATEDIR=/run "-GUnix Makefiles" \
	-DCMAKE_INSTALL_LIBDIR="/usr/lib/${HOST_MULTIARCH}" \
	..

cmake --build . --parallel


# -- Derive shared-library runtime dependencies from the installed plugin.

PACKAGE_ROOT=$(mktemp -d)
trap 'rm -rf "$PACKAGE_ROOT"' EXIT

DESTDIR="$PACKAGE_ROOT" cmake --install .

MODULE_PATH="$PACKAGE_ROOT/usr/lib/${HOST_MULTIARCH}/calamares/modules/partitionq/libcalamares_viewmodule_partitionq.so"
if [ ! -f "$MODULE_PATH" ]; then
	echo "The partitionq plugin was not installed at the expected path: $MODULE_PATH" >&2
	exit 1
fi

SHLIB_DEPENDENCIES=$(dpkg-shlibdeps -O "$MODULE_PATH" \
	| sed -n 's/^shlibs:Depends=//p')
if [ -z "$SHLIB_DEPENDENCIES" ]; then
	echo "Unable to derive shared-library dependencies for the partitionq plugin." >&2
	exit 1
fi
PACKAGE_DEPENDENCIES="calamares (>= 3.4.2), calamares (<< 3.5~)"
if [ -n "$SHLIB_DEPENDENCIES" ]; then
	PACKAGE_DEPENDENCIES="$PACKAGE_DEPENDENCIES, $SHLIB_DEPENDENCIES"
fi




# -- Run checkinstall and Build Debian Package

printf "%s\n" \
	"Calamares QML modules for Nitrux." \
	"" \
	"Qt 6 QML partition view module and backend for Calamares." \
	"" > description-pak

checkinstall -D -y \
	--install=no \
	--fstrans=yes \
	--pkgname=calamares-modules-nitrux \
	--pkgversion="${PACKAGE_VERSION:-0.1.0}" \
	--pkgarch="${TARGET_ARCH:-$(dpkg --print-architecture)}" \
	--pkgrelease="1" \
	--pkglicense=GPL-3 \
	--pkggroup=utils \
	--pkgsource=calamares-modules-nitrux \
	--pakdir=. \
	--maintainer=uri_herrera@nxos.org \
	--provides=calamares-modules-nitrux \
	--requires="$PACKAGE_DEPENDENCIES" \
	--nodoc \
	--strip=no \
	--stripso=yes \
	--reset-uids=yes \
	--deldesc=yes \
	-- cmake --install .
