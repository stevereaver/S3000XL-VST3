---
type: Policy
title: Contribution Conventions
description: Coding, branching, and verification conventions for contributors and agents working on S3000XL-VST3.
resource: ../README.md
tags: [policy, contribution, git, workflow, a2a, verification]
generated:
  by: agent/devin
  at: 2026-09-09T21:30:00Z
verified: []
status: stable
stale_after: 2027-03-09
sources:
  - id: readme
    resource: ../../README.md
    title: S3000XL-VST3 README
    author: human:steve
    last_modified: 2026-09-09
  - id: recipe
    resource: ../../MAME-to-VSTi-RECIPE.md
    title: MAME → VSTi conversion recipe
    author: human:steve
    last_modified: 2026-07-26
---

# Contribution Conventions

## A2A discipline

- **The repo/source is the judge.** Grep before claiming a symbol exists or a behavior is implemented.
- **The compiler and the DAW are the final judges.** Standalone MAME tests and real DAW verification beat assumptions.
- **The chat can be lost; the repo cannot.** Document important findings in `STATE.json`, `BUILD.md`, or this OKF bundle.

## Branches

| Branch | Purpose |
|--------|---------|
| `main` | Stable releases; merges come only from `dev` |
| `dev` | Integration branch for day-to-day development and CI testing |

Feature branches target `dev`. When `dev` is green, open a PR to `main`; merging to `main` creates the release pipeline.

## Before changing code

1. Reproduce or understand the current behavior with the existing build.
2. For MAME-side changes, verify the machine still boots standalone after patches are applied.
3. For audio/render changes, capture a WAV via `mame s3000xl -wavwrite t.wav -seconds_to_run 6` and inspect the header.
4. For UI changes, test in a real DAW when possible; JUCE's standalone wrapper is not a substitute for host-specific behavior.

## Patch management

- MAME modifications must live in `patches/mame-0288-*.patch`.
- Patches are applied in sorted order; name new patches so they apply after dependencies.
- Each patch should be self-contained and include a header comment explaining why it is needed.
- Do not modify `mame/` directly and commit it; `mame/` is not part of the repo.

## Code style

- Follow the existing JUCE/MAME style in the files you touch.
- Keep MAME macros (`PTR64`, `LSB_FIRST`, `NDEBUG`, etc.) defined before any MAME includes.
- Prefer atomic flags and lock-free ring buffers for audio/MAME thread communication.
- Do not introduce static state in `processBlock`; it breaks multi-instance isolation.

## ROM and media policy

- Never commit firmware ROMs, disk images, or copyrighted sample content.
- CI verifies that build artifacts contain none of: `.bin`, `.chd`, `.iso`, `.img`, `.hfe`, `.eeprom`, `.nrg`, `.cue`.
- The plugin must remain non-functional without user-supplied ROMs.

## Verification checklist for pull requests

- [ ] Local build passes: `scripts\build-mame-libs.bat` and `scripts\build-plugin.ps1 -Config Release`
- [ ] No ROMs or disk images in the diff
- [ ] New/changed MAME patches apply cleanly to `mame0288`
- [ ] Plugin loads in a DAW and the panel is not black (ROMs present)
- [ ] Audio output is clean stereo (no stride/channel mapping regressions)
- [ ] `STATE.json` or OKF bundle updated if a key finding or open item changed
