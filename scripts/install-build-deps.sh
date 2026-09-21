#!/usr/bin/env bash

# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024-2025 <Nitrux Latinoamericana S.C. <hello@nxos.org>>


# -- Exit on errors.

set -euo pipefail


# -- Check if running as root.

if [ "$EUID" -ne 0 ]; then
    APT_COMMAND="sudo apt"
else
    APT_COMMAND="apt"
fi


# -- Install build packages.

$APT_COMMAND update -q
$APT_COMMAND install -y --no-install-recommends \
    appstream \
    automake \
    autotools-dev \
    build-essential \
    checkinstall \
    clang \
    cmake \
    curl \
    devscripts \
    dpkg-dev \
    equivs \
    extra-cmake-modules \
    gettext \
    gnupg2 \
    libkf6coreaddons-dev \
    libkf6i18n-dev \
    libkf6widgetsaddons-dev \
    libkpmcore-dev \
    lintian \
    qt6-base-dev \
    qt6-declarative-dev
