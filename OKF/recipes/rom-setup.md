---
type: Recipe
title: ROM Setup and Self-Check
description: Required ROM files for the S3000XL machine, where to place them, and how the plugin validates them before booting MAME.
resource: ../plugin/Source/PluginProcessor.cpp
tags: [roms, firmware, self-check, mame, s3000xl, hd61830]
generated:
  by: agent/devin
  at: 2026-09-09T21:30:00Z
verified: []
status: stable
stale_after: 2027-03-09
sources:
  - id: state
    resource: ../../STATE.json
    title: Project state and ROM manifest
    author: human:steve
    last_modified: 2026-07-26
  - id: port-map
    resource: ../../S3000XL-PORT-MAP.md
    title: S3000XL port map
    author: human:steve
    last_modified: 2026-07-26
  - id: source-processor
    resource: ../../plugin/Source/PluginProcessor.cpp
    title: PluginProcessor.cpp — ROM verification and boot path
    author: agent/devin
    last_modified: 2026-09-09
---

# ROM Setup and Self-Check

The plugin does not ship ROMs. The user must provide the firmware for the emulated `s3000xl` machine.

## Required files

Place the following files in `Documents\S3000XL\ROMs\s3000xl\`:

| File | Size | Region | Purpose |
|------|------|--------|---------|
| `s3000xl_v2_0.bin` | 512 KB (`0x80000`) | `maincpu` | S3000XL v2.0 BIOS |
| `hd61830.bin` | 1472 bytes | device LCD char-gen | HD61830 / LC7981 character generator |

Known good hashes for the main BIOS:

- CRC: `26a00ddd`
- SHA1: `ace009953aeccff4dfbee0ccfa091a0fea39a386`

Alternative BIOS versions exist (`v1.50`, `v1.06`) but v2.0 is the default.

## ROM verification

`verifyRomFiles()` checks that the required files are present and readable. If ROMs are missing:

- `isRomMissing` is set to `true`.
- The plugin disables the main UI and shows a **Locate sd132.zip** / **Rescan** button.
- The user can either point to a zip containing the ROMs or to a folder; the plugin extracts/copies only the required `.bin` files into the canonical `Documents\S3000XL\ROMs\s3000xl\` folder.

If extraction/copying fails or the expected files are still missing:

- `isRomInvalid` is set to `true`.
- A self-check failure message is shown instead of the panel.

## The black panel trap

Missing `hd61830.bin` is the most common cause of a **black panel**. MAME fails to initialize the machine silently, so the render list is empty. The plugin's self-check is designed to catch this before attempting to boot and present a clear error instead of a blank UI.

## Self-check sequence

1. Verify ROM files exist and are non-empty.
2. If missing, show ROM locator UI.
3. If present, boot MAME on a background thread via `runMameEngine()`.
4. If `cli_frontend::execute()` returns non-zero, set `isMameRunning = false` and clear any persisted media paths to avoid a crash loop.
5. Once the first frame is delivered, mark `mameIsFullyBooted = true`.

## Manual verification with standalone MAME

Before relying on the plugin, you can test the machine directly:

```batch
mame s3000xl -verifyroms
mame s3000xl -wavwrite t.wav -seconds_to_run 6
```

The second command confirms the machine boots and lets you verify the audio stride via the WAV header.
