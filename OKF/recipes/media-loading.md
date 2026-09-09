---
type: Recipe
title: Media Loading — Floppy, CD, and HDD Images
description: How to load and unload floppy, CD, and SCSI hard-disk images into the running S3000XL machine, and the MAME device-timing rules that affect sample integrity.
resource: ../plugin/Source/PluginProcessor.cpp
tags: [media, floppy, cdrom, hdd, scsi, sample-loading, mame]
generated:
  by: agent/devin
  at: 2026-09-09T21:30:00Z
verified: []
status: stable
stale_after: 2027-03-09
sources:
  - id: state
    resource: ../../STATE.json
    title: Project state file
    author: human:steve
    last_modified: 2026-07-26
  - id: recipe
    resource: ../../MAME-to-VSTi-RECIPE.md
    title: MAME → VSTi conversion recipe
    author: human:steve
    last_modified: 2026-07-26
  - id: source-processor
    resource: ../../plugin/Source/PluginProcessor.cpp
    title: PluginProcessor.cpp — media request logic
    author: agent/devin
    last_modified: 2026-09-09
---

# Media Loading — Floppy, CD, and HDD Images

The emulated S3000XL exposes three kinds of removable media: floppy disk, SCSI CD-ROM, and SCSI hard disk. The plugin UI provides a **MEDIA** button that queues load/unload requests for the MAME thread.

## Supported formats

| Media | Extensions | Notes |
|-------|------------|-------|
| Floppy | `.img`, `.hfe`, `.dsk`, etc. | Handled by `upd72069_device` FDC |
| CD-ROM | `.iso`, `.cue`/`.bin`, `.chd` | SCSI ID 4; use `cdrom_2x` timing |
| HDD | `.chd` | SCSI ID 5; opened read-write exclusive |

## SCSI defaults in the driver

The `s3000xl` MAME driver pre-populates SCSI slots:

- `scsi:4 = cdrom_2x`
- `scsi:5 = harddisk`

Do not blindly override these slots. The plugin uses the driver defaults and only loads an image into the already-attached device.

## Critical CD-ROM timing rule

`cdrom` and `cdrom_2x` share the same SCSI identity (Sony CDU-76S) but differ **only** in data timing:

- `cdrom_2x` transfers at ~307200 B/s with a 100 µs read-command delay.
- Plain `cdrom` is too fast for the firmware and causes **corrupted sample data → audible crackle**.

Rule of thumb:

- When a CD image is loaded, the device must be `cdrom_2x`.
- When no CD is loaded, plain `cdrom` may be used so an idle 2× drive does not stall the main loop with spin-up retries.

HDD and `cdrom_2x` images load cleanly in testing.

## Hard disk file locking

MAME opens `.chd` files **read-write exclusive**. If another MAME process (e.g., standalone MAMEUI) has the same disk open, the plugin receives:

```text
permission denied (generic:13)
```

This is a fatal error. Do not run standalone MAME on the same disk while testing the plugin.

## UI workflow

1. User clicks the **MEDIA** button.
2. A `juce::FileChooser` opens for the selected media category.
3. The selected path is stored in the appropriate atomic pending path:
   - `pendingFloppyPath` + `requestFloppyLoad`
   - `pendingCdPath` + `requestCdLoad`
   - `pendingHddPath` + `requestHddLoad`
4. The MAME thread detects the request and mounts the image through MAME's device image interface.
5. State is persisted in `fileManagerState` so it survives editor destroy/recreate.

## Graceful boot failure

If MAME fails to boot (e.g., missing ROM or locked media), the plugin must:

1. Set `isMameRunning = false` so the audio thread does not wait forever.
2. Clear persisted `scsi_hdd_path` / `scsi_cd_path` and save state so the bad path is not reloaded next launch.

Failure to do either can freeze the DAW or put the plugin into a crash loop.
