# S3000XL-VST3

An independent, open-source **VST3** instrument built around a headless MAME emulation of the Akai S3000XL 16-bit rack sampler.

This repository is a clean, CMake-based fork of the original [S3000XL-VST](https://github.com/Tuth/S3000XL-VST) project. It keeps the plugin source and the required MAME patches, but replaces the vendored JUCE/MAME trees and absolute-path Projucer build with a modern, CI-friendly CMake workflow.

> **No ROMs or firmware are included.** The plugin is non-functional without user-supplied firmware.

## Branches

| Branch | Purpose |
|--------|---------|
| `main` | Stable releases. Merges come from `dev` only. |
| `dev`  | Integration branch for day-to-day development and CI testing. |

Feature branches should target `dev`. When `dev` is green, open a PR to `main`; merging to `main` creates the release pipeline.

## Quick start (local Windows build)

1. Install the prerequisites:
   - Visual Studio 2022 with the **Desktop development with C++** workload
   - [MSYS2](https://www.msys2.org/) (used only to run GENie)
   - CMake 3.22+

2. Clone this repo and the dependencies:
   ```powershell
   git checkout dev
   scripts\setup-dependencies.ps1
   scripts\apply-mame-patches.ps1
   ```

3. Build the MAME static libraries:
   ```batch
   scripts\build-mame-libs.bat
   ```

4. Build the plugin:
   ```powershell
   scripts\build-plugin.ps1 -Config Release
   ```

The generated VST3 bundle will be under `build\S3000XL-VST_artefacts\Release\`.

For more details, see [BUILD.md](BUILD.md).

## CI / CD

GitHub Actions are defined in `.github/workflows`:

- `ci.yml` runs on every push/PR to `dev` or `main`. It clones JUCE and MAME, applies patches, builds the MAME libs (cached), and builds the VST3 plugin.
- `release.yml` triggers on tags `v*` and creates a draft GitHub release containing the zipped VST3 bundle.

## Legal

The plugin is distributed under **GPLv3** because it combines MAME (GPL-2.0-or-later) and JUCE (GPLv3 terms). MAME modifications are shipped as patches in the `patches/` directory, as the GPL requires.

This project is **not affiliated with, endorsed by, or sponsored by** any hardware manufacturer. Product and company names are the trademarks of their respective owners and are used only to identify the hardware being emulated.
