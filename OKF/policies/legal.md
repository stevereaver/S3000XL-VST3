---
type: Policy
title: Legal and Distribution Policy
description: Licensing, ROM/disk distribution rules, and trademark guidelines for the S3000XL-VST3 project.
resource: ../LICENSE
tags: [policy, legal, gpl, roms, trademark, distribution]
generated:
  by: agent/devin
  at: 2026-09-09T21:30:00Z
verified: []
status: stable
stale_after: 2027-03-09
sources:
  - id: license
    resource: ../../LICENSE
    title: GPLv3 license
    author: human:steve
    last_modified: 2026-08-08
  - id: readme
    resource: ../../README.md
    title: S3000XL-VST3 README
    author: human:steve
    last_modified: 2026-09-09
  - id: state
    resource: ../../STATE.json
    title: Project state file
    author: human:steve
    last_modified: 2026-07-26
---

# Legal and Distribution Policy

## License

The plugin is distributed under **GPLv3**.

- **MAME** is licensed under **GPL-2.0-or-later**.
- **JUCE** is used under its **GPLv3** terms.

Because the plugin combines both codebases, the combined work is released under GPLv3. A copy of the license is included in the repository as `LICENSE`.

## ROM and firmware policy

- **No ROMs or firmware are included in the repository, the CI artifacts, or the release zip.**
- The plugin is intentionally non-functional without user-supplied firmware.
- Required ROM files are listed in `recipes/rom-setup.md` and must be obtained legally by the end user.
- Do not commit, attach, or link to ROM download sources in issues, pull requests, or documentation.
- CI verifies that build artifacts do not contain `.bin`, `.chd`, `.iso`, `.img`, `.hfe`, `.eeprom`, `.nrg`, or `.cue` files.

## MAME patches and source compliance

- Modifications to MAME are shipped as patches in `patches/`.
- This satisfies the GPL requirement that corresponding source be made available for any distributed binaries built from modified MAME source.
- The patches are applied to the upstream `mame0288` tag.

## Trademarks

- **Akai**, **S3000XL**, and any other hardware/vendor names are trademarks of their respective owners.
- This project is **not affiliated with, endorsed by, or sponsored by** any hardware manufacturer.
- Product and company names are used only to identify the hardware being emulated.
- The on-screen panel showing "AKAI S3000XL" is MAME's own hardware artwork and constitutes nominative use.

## Repository naming

The repository uses a brand-neutral name (`S3000XL-VST`) rather than a trademark-heavy name, to reduce confusion about official affiliation.

## History note

The local `master` branch history once contained a firmware ROM blob. That blob was removed, but the history still contains it. The public repository was created from an orphan branch (`public`, pushed as `origin/main`) so the ROM object is not reachable from any published history. Future publishing must continue from the cleaned history. If `master` is ever published, the blob must be purged first (for example with `git filter-repo`).
