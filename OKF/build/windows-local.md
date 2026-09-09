---
type: Build
title: Local Windows Build
description: Step-by-step instructions for building the S3000XL-VST3 plugin on a Windows developer machine.
resource: ../BUILD.md
tags: [build, windows, msvc, msys2, cmake, mame, juce]
generated:
  by: agent/devin
  at: 2026-09-09T21:30:00Z
verified: []
status: stable
stale_after: 2027-03-09
sources:
  - id: build-doc
    resource: ../../BUILD.md
    title: BUILD.md
    author: human:steve
    last_modified: 2026-09-09
  - id: root-cmake
    resource: ../../CMakeLists.txt
    title: Root CMakeLists.txt
    author: human:steve
    last_modified: 2026-09-09
  - id: scripts
    resource: ../../scripts/
    title: Build helper scripts
    author: human:steve
    last_modified: 2026-09-09
---

# Local Windows Build

## Prerequisites

- Windows 10/11
- Visual Studio 2022 with the **Desktop development with C++** workload
- [MSYS2](https://www.msys2.org/) installed (default `C:\msys64`)
- CMake 3.22 or newer
- Git for Windows

Optional environment overrides:

| Variable | Default |
|----------|---------|
| `JUCE_ROOT` | `<repo>\JUCE` |
| `MAME_ROOT` | `<repo>\mame` |
| `MSYS2_ROOT` | `C:\msys64` |
| `VS_VCVARSALL` | detected by `build-mame-libs.bat` |

## Build steps

### 1. Check out the integration branch

```powershell
git checkout dev
```

### 2. Clone dependencies

```powershell
scripts\setup-dependencies.ps1
```

This clones shallow copies of:

- `JUCE` at tag `8.0.15`
- `mame` at tag `mame0288`

### 3. Apply MAME patches

```powershell
scripts\apply-mame-patches.ps1
```

All `patches\mame-0288-*.patch` files are applied in sorted order.

### 4. Build the MAME static libraries

```batch
scripts\build-mame-libs.bat
```

This runs two phases:

1. GENie under MSYS2/MinGW64:
   `make vs2022 SUBTARGET=s3000xl SOURCES=src/mame/akai/s3000.cpp NOWERROR=1`
2. MSBuild with the VS2022 x64 toolset:
   `mame\build\projects\windows\mames3000xl\vs2022\mames3000xl.sln`

Logs are written to `mame\genie_s3000xl.log` and `mame\msbuild_mame_s3000xl.log`.

Output libs land in `mame\build\vs2022\bin\x64\Release\` and `mame\build\vs2022\bin\x64\Release\mame_s3000xl\`.

### 5. Build the plugin

```powershell
scripts\build-plugin.ps1 -Config Release
```

Equivalent CMake commands:

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DJUCE_ROOT=".\JUCE" -DMAME_ROOT=".\mame"
cmake --build build --config Release --target S3000XL-VST_VST3 --parallel
```

### 6. Install / test

The VST3 bundle is at:

```text
build\plugin\S3000XL-VST_artefacts\Release\VST3\S3000XL-VST.vst3
```

Copy it to your DAW's VST3 scan path or to `C:\Program Files\Common Files\VST3`.

You must provide your own ROMs in `Documents\S3000XL\ROMs\s3000xl\` before the plugin will boot.

## Common issues

- **Toolset mismatch:** the MAME libs and plugin must use the same MSVC ABI. The helper scripts use the same VS environment, so this is handled automatically.
- **Missing `hd61830.bin`:** the LCD character-generator device ROM is required; without it the panel is black.
- **`git apply` fails:** make sure `mame\` is exactly at the `mame0288` tag with no prior patches applied.
- **Out of disk space:** the MAME build is large; ensure at least 20 GB free on the build drive.
- **`LNK1105` / locked lib:** do not kill processes mid-build. Let the build finish, then retry. The DAW holding the old `.vst3` also blocks the post-build copy.
