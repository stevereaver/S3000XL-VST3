---
type: Build
title: CI/CD Workflows
description: GitHub Actions workflows that build, verify, and release the S3000XL-VST3 plugin.
resource: ../.github/workflows/ci.yml
tags: [ci, cd, github-actions, release, artifact, mame, juce]
generated:
  by: agent/devin
  at: 2026-09-09T21:30:00Z
verified: []
status: stable
stale_after: 2027-03-09
sources:
  - id: ci-yml
    resource: ../../.github/workflows/ci.yml
    title: CI workflow
    author: human:steve
    last_modified: 2026-09-09
  - id: release-yml
    resource: ../../.github/workflows/release.yml
    title: Release workflow
    author: human:steve
    last_modified: 2026-09-09
---

# CI/CD Workflows

Two GitHub Actions workflows live in `.github/workflows`.

## CI workflow (`ci.yml`)

Triggers:

- push or pull request to `dev` or `main`
- `workflow_dispatch`

Runner: `windows-2022`
Timeout: 360 minutes

Steps:

1. Checkout repo.
2. Set up MSYS2 (`MINGW64`) with `git`, `make`, `mingw-w64-x86_64-gcc`, `mingw-w64-x86_64-python`.
3. Clone JUCE `8.0.15` into `./JUCE`.
4. Clone MAME `mame0288` into `./mame`.
5. Apply all `patches/*.patch` files to `mame/`.
6. Restore `mame/build/vs2022/bin/x64/Release` from `actions/cache@v4` keyed on OS, MAME tag, and patch hash.
7. If cache miss:
   - Run `make vs2022 SUBTARGET=s3000xl SOURCES=src/mame/akai/s3000.cpp NOWERROR=1` under MSYS2.
   - Build the generated `mames3000xl.sln` with MSBuild in `Release/x64`, tool architecture `x64`, with `_CL_` override `WIN32_WINNT=0x0A00` and `NTDDI_VERSION=0xA000000`.
8. Configure the plugin with CMake.
9. Build target `S3000XL-VST_VST3`.
10. Upload the built `S3000XL-VST.vst3` as an artifact.
11. Verify the artifact tree contains **no** ROMs or disk images (`.bin`, `.chd`, `.iso`, `.img`, `.hfe`, `.eeprom`, `.nrg`, `.cue`).

## Release workflow (`release.yml`)

Triggers:

- push of a tag matching `v*`

Runner: `windows-2022`
Timeout: 360 minutes

Steps mirror CI, but after the build:

1. Package the `.vst3` bundle into `S3000XL-VST3-<tag>-win64/`.
2. Zip the folder as `S3000XL-VST3-<tag>-win64.zip`.
3. Create a draft GitHub release with the zip attached and auto-generated release notes.

## Important CI constraints

- MAME libraries are cached only if the exact same patch set is present. Adding or changing a patch invalidates the cache.
- The `_CL_` NTDDI override is applied **only** during the MAME library build, not the plugin build.
- CI does not run the plugin (no ROMs), so functional tests must be performed locally.
- Release artifacts are **draft**; a human must publish them after review.

## Branch strategy

- `main`: stable releases; merges come only from `dev`.
- `dev`: integration branch; feature branches target `dev`.
