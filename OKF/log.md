---
type: Log
title: S3000XL-VST3 OKF Bundle Changelog
description: Changes to this knowledge bundle.
resource: ./index.md
tags: [okf, changelog, bundle]
generated:
  by: agent/devin
  at: 2026-09-09T21:30:00Z
verified: []
status: stable
stale_after: 2027-03-09
---

# S3000XL-VST3 OKF Bundle Changelog

## 1.0.0 — 2026-09-09

Initial bundle created from current project state.

### Added

- `index.md` — bundle overview, layout, and project invariants
- `architecture/audio-pipeline.md` — MAME → VST3 audio streaming, stride, throttling
- `architecture/render-pipeline.md` — MAME render target, LCD live-refresh fix, double buffering
- `architecture/input-handling.md` — panel button matrix, DATA ENTRY dial, MIDI in/out
- `build/windows-local.md` — local Windows build steps and prerequisites
- `build/ci-cd.md` — GitHub Actions CI and release workflows
- `recipes/rom-setup.md` — required ROMs, self-check, and black-panel trap
- `recipes/media-loading.md` — floppy/CD/HDD media loading and SCSI timing rules
- `policies/contribution.md` — A2A discipline, branch strategy, patch management, PR checklist
- `policies/legal.md` — GPLv3, ROM distribution rules, trademark policy, history note

### OKF version

Written against **Open Knowledge Format v0.2**. All concepts use YAML frontmatter with `type`, `title`, `description`, `resource`, `tags`, `generated`, `verified`, `status`, and `stale_after`. Sources are declared in `sources` and cited in body text with `[^source-id]` footnotes where applicable.

### Open items captured

- LCD render pump residual stalls under heavy frameskip.
- Maximum RAM confirmation (target 32 MiB, fallback 16 MiB).
- Multi-instance feasibility (MAME process-global state risk).
