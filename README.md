# Calamares Modules for Nitrux | [![License](https://img.shields.io/badge/License-GPL_3-green.svg)](https://opensource.org/license/gpl-3.0)

Additional Calamares modules for Nitrux.

# Introduction

This repository provides the standalone `partitionq` Calamares view-module. It reuses the non-Widgets partitioning backend from the Calamares revision used by Nitrux and exposes that backend to a QML view through `QmlViewStep`.

The module is intentionally separate from the Calamares package. It does not replace or overwrite Calamares' `partition` plugin. The QML view is supplied by the Nitrux Calamares settings package, which allows the interface to use the Nitrux MauiKit controls without putting branding code in the backend package.

## Building

The build requires the Calamares development package built with QML support, KPMCore, the matching Qt major version, and the matching KDE Frameworks components:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build
```

The generated plugin is installed in the Calamares module directory as:

```text
calamares/modules/partitionq/module.desc
calamares/modules/partitionq/libcalamares_viewmodule_partitionq.so
```

> [!IMPORTANT]
> The settings package must provide `partitionq.conf` and the QML file selected by `qmlFilename`.

# Licensing

This repository contains files under multiple licenses.

- Repository build and packaging automation is licensed under **BSD-3-Clause** (see `LICENSE`).
- The vendored backend sources are derived from Calamares and retain their original SPDX headers and licensing: **GPL-3.0-or-later**.

# Issues

If you find problems with the contents of this repository, please create an issue and use the **🐞 Bug report** template.

## Submitting a bug report

Before submitting a bug, you should look at the [existing bug reports](https://github.com/Nitrux/calamares-modules-nitrux/issues) to verify that no one has reported the bug already.

©2026 Nitrux Latinoamericana S.C.
