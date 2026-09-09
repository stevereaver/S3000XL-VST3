# Building S3000XL-VST3

This document explains how to build the plugin locally on Windows.

## Repository layout

```text
S3000XL-VST3/
├── plugin/              # JUCE plugin source + resources
│   ├── CMakeLists.txt
│   └── Source/
├── patches/             # MAME 0.288 patches
├── scripts/             # Local build helpers
├── .github/workflows/    # CI/CD
├── CMakeLists.txt       # Root CMake
└── CMakePresets.json    # Optional preset for VS/CMake GUI
```

`JUCE/` and `mame/` are **not committed**. They are cloned on demand by the helper scripts or by CI.

## Prerequisites

- Windows 10/11
- Visual Studio 2022 with the **Desktop development with C++** workload
- [MSYS2](https://www.msys2.org/) installed, e.g. at `C:\msys64`
- CMake 3.22 or newer
- Git for Windows

## Environment variables (optional)

| Variable | Purpose | Default |
|----------|---------|---------|
| `JUCE_ROOT` | Path to a JUCE 8.0.15 source tree | `<repo>\JUCE` |
| `MAME_ROOT` | Path to a MAME 0.288 source tree | `<repo>\mame` |
| `MSYS2_ROOT` | Path to MSYS2 | `C:\msys64` |
| `VS_VCVARSALL` | Path to `vcvarsall.bat` | Detected from `build-mame-libs.bat` default |

## Step-by-step

### 1. Check out `dev`

```powershell
git checkout dev
```

### 2. Clone dependencies

```powershell
scripts\setup-dependencies.ps1
```

This clones JUCE `8.0.15` and MAME `mame0288` as shallow repositories into the repo root.

### 3. Apply MAME patches

```powershell
scripts\apply-mame-patches.ps1
```

All `patches\mame-0288-*.patch` files are applied to `mame/`.

### 4. Build the MAME static libraries

```batch
scripts\build-mame-libs.bat
```

This performs two steps that mirror the original `.bat` files:

1. Runs `make vs2022 SUBTARGET=s3000xl SOURCES=src/mame/akai/s3000.cpp NOWERROR=1` under MSYS2/MinGW64 to generate the Visual Studio solution.
2. Builds the solution with `msbuild` in `Release`/`x64` configuration.

Output lands in `mame\build\vs2022\bin\x64\Release\`.

### 5. Build the plugin

With the helper script:

```powershell
scripts\build-plugin.ps1 -Config Release
```

Or with CMake presets:

```powershell
cmake --preset windows-release
cmake --build build --config Release --target S3000XL-VST_VST3 --parallel
```

The VST3 bundle is written to `build\plugin\S3000XL-VST_artefacts\Release\VST3\S3000XL-VST.vst3`.

### 6. Install / test

Copy the `.vst3` bundle into your DAW's plugin scan path or into `C:\Program Files\Common Files\VST3`.

Remember: you must provide your own firmware ROMs in `Documents\S3000XL\ROMs\s3000xl\` for the plugin to boot.

## Troubleshooting

- **Toolset mismatch:** The MAME libs and the plugin must be built with the same MSVC ABI. Both local scripts use the same Visual Studio environment, so this is handled automatically.
- **Missing `hd61830.bin`:** The LCD character-generator ROM is required; without it the panel will be black.
- **`git apply` fails on patches:** Make sure `mame/` is exactly at the `mame0288` tag.
- **Out of disk space during MAME build:** The MAME build is large. Ensure at least 20 GB of free space on the build drive.
