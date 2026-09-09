---
type: OKF Bundle
title: S3000XL-VST3 Knowledge Bundle
description: Agent-readable context for the S3000XL-VST3 project — a VST3 instrument wrapping a headless MAME emulation of the Akai S3000XL sampler.
resource: https://github.com/Tuth/S3000XL-VST
tags: [s3000xl, vst3, mame, juce, sampler, emulation, c++, windows]
version: 1.0.0
okf_version: 0.2
generated:
  by: agent/devin
  at: 2026-09-09T21:30:00Z
verified: []
status: stable
stale_after: 2027-03-09
sources:
  - id: repo-root
    resource: ../README.md
    title: S3000XL-VST3 README
    author: human:steve
    last_modified: 2026-09-09
  - id: build-doc
    resource: ../BUILD.md
    title: Local Windows build instructions
    author: human:steve
    last_modified: 2026-09-09
  - id: state-json
    resource: ../STATE.json
    title: Project state and findings
    author: human:steve
    last_modified: 2026-07-26
  - id: recipe-doc
    resource: ../MAME-to-VSTi-RECIPE.md
    title: MAME → VSTi conversion recipe
    author: human:steve
    last_modified: 2026-07-26
---

# S3000XL-VST3 Knowledge Bundle

This bundle captures the architecture, build process, operational recipes, and conventions for the **S3000XL-VST3** project. It is intended for AI agents and human contributors who need to understand or modify the code without relying on chat history.

## Scope

- VST3 instrument plugin built with JUCE 8.0.15
- Headless MAME 0.288 core driving an `s3000xl` machine definition
- Windows primary target (Visual Studio 2022, MSYS2/MinGW64 for GENie)
- GPLv3 distribution; no ROMs or firmware are shipped

## Layout

```text
OKF/
├── index.md                         # this file
├── log.md                           # bundle changelog
├── architecture/
│   ├── audio-pipeline.md            # MAME → VST audio streaming
│   ├── render-pipeline.md           # LCD/panel rendering and MAME frames
│   └── input-handling.md            # Buttons, DATA ENTRY dial, MIDI
├── build/
│   ├── windows-local.md             # Local Windows build steps
│   └── ci-cd.md                     # GitHub Actions workflows
├── recipes/
│   ├── rom-setup.md                 # Required ROMs and self-check
│   └── media-loading.md             # Floppy, CD, HDD image loading
└── policies/
    ├── contribution.md              # Contribution conventions
    └── legal.md                     # GPL, ROMs, trademark rules
```

## How to use this bundle

1. Start with the relevant section for your task (build, audio, input, etc.).
2. Trust signals are carried in frontmatter: `generated`, `verified`, `status`, `stale_after`.
3. For implementation details, follow the `resource` links back to source files and original docs.

## Important project invariants

- **No ROMs are committed.** The plugin requires user-supplied firmware under `Documents\S3000XL\ROMs\s3000xl\`.
- **MAME static libs are not committed.** They are built from the patched `mame/` clone on demand.
- **Source of truth is the repo.** Grep before claiming a symbol exists; the compiler and the DAW are the final judges.
- **Unique plugin identity.** The S3000XL plugin uses 4-char code `S3kx`; do not reuse `Mp30` or other machine codes.
