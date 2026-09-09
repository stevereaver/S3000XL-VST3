# S3000XL-VST3 User Manual

S3000XL-VST3 is a VST3 instrument plugin that runs a complete, headless emulation of the Akai S3000XL 16-bit rack sampler inside your DAW. The emulation is powered by MAME and wrapped with JUCE, giving you the original firmware experience — LCD panel, front-panel buttons, data-entry dial, MIDI, and SCSI media — directly in your project.

> **Important:** No ROMs or firmware are included. The plugin requires user-supplied firmware files to function.

---

## Table of Contents

1. [Installation](#installation)
2. [ROM Setup](#rom-setup)
3. [First Launch](#first-launch)
4. [Panel Overview](#panel-overview)
5. [Loading Samples and Media](#loading-samples-and-media)
6. [MIDI](#midi)
7. [Plugin Settings](#plugin-settings)
8. [DAW State and Recall](#daw-state-and-recall)
9. [Troubleshooting](#troubleshooting)

---

## Installation

### From a release build

1. Download the latest release `.zip` from the [Releases](https://github.com/stevrob790/S3000XL-VST3/releases) page.
2. Extract the `S3000XL-VST.vst3` bundle.
3. Copy it into your DAW's VST3 scan path. The standard Windows location is:

   ```
   C:\Program Files\Common Files\VST3\
   ```

4. Rescan plugins in your DAW. The plugin will appear as **S3000XL-VST**.

### From source

See [BUILD.md](BUILD.md) for full build instructions. The output `.vst3` bundle is located at:

```
build\plugin\S3000XL-VST_artefacts\Release\VST3\S3000XL-VST.vst3
```

---

## ROM Setup

The plugin emulates the real S3000XL hardware and needs the original firmware ROM files to boot. These are **not included** and must be obtained legally by the user.

### Required ROM files

| File | Size | Description |
|------|------|-------------|
| `s3000xl_v2_0.bin` | 512 KB | S3000XL v2.0 main CPU BIOS |
| `hd61830.bin` | 1,472 bytes | HD61830 LCD character-generator ROM |

### Where to place them

Create the following folder structure under your Windows **Documents** folder:

```
Documents\
  S3000XL\
    ROMs\
      s3000xl\
        s3000xl_v2_0.bin
        hd61830.bin
```

The full path is typically:

```
C:\Users\<YourName>\Documents\S3000XL\ROMs\s3000xl\
```

### Alternative: using a zip file

Instead of placing loose `.bin` files, you can drop a `.zip` archive containing the ROM files into any of these locations and the plugin will extract them automatically:

- `Documents\S3000XL\sd132.zip` (legacy location)
- `Documents\S3000XL\ROMs\sd132.zip`

The plugin scans for the required `.bin` files inside the zip and extracts only the ones it needs into the correct `ROMs\s3000xl\` subfolder.

> **Note:** `.7z` archives are not supported. If you have a `.7z` file, extract it manually with 7-Zip or a similar tool first.

### Using the ROM locator

If ROM files are missing when the plugin loads, it will display an orange warning screen listing the missing files. Two buttons are provided:

- **Locate ROMs...** — Opens a file browser. You can point it to a `.zip` file, a `.bin` file, or a folder containing the ROMs. The plugin will copy or extract the required files automatically.
- **Rescan ROMs Folder** — Re-checks the `Documents\S3000XL\ROMs\s3000xl\` folder after you have placed files there manually.

### Alternative BIOS versions

The default firmware is v2.0. Alternative versions (v1.50, v1.06) exist but must be renamed to `s3000xl_v2_0.bin` to be recognized, or used with standalone MAME for testing. v2.0 is recommended.

---

## First Launch

1. Open your DAW and insert **S3000XL-VST** as a virtual instrument on a MIDI track.
2. If ROMs are correctly installed, the emulated S3000XL panel will appear after a few seconds of boot time (the emulated machine goes through its startup sequence, just like the real hardware).
3. If the panel is black or you see an orange warning, see [ROM Setup](#rom-setup) above.

The plugin boots the full S3000XL firmware. You will see the familiar LCD display and can interact with all front-panel controls.

---

## Panel Overview

The plugin renders the actual MAME artwork for the S3000XL front panel. You interact with it using your mouse:

### Buttons

Click directly on any front-panel button to activate it. The full button set is available:

- **Mode buttons:** Load, Save, Global, Edit, Effects, Sample, Multi, Single
- **Soft keys:** F1 through F8
- **Numeric keypad:** 0-9, +, -
- **Navigation:** Up, Down, Left, Right arrows
- **Editing:** Enter/Play, Name, Mark, Jump

### DATA ENTRY dial

The large data-entry knob can be controlled in several ways:

- **Click the +/- zones** around the dial for single-step increments
- **Click and drag** vertically on the dial to turn it
- The dial is also exposed as the `data_entry` VST parameter (0.0 to 1.0), which can be automated from your DAW

> **Note:** The dial has a known MAME-side quirk on Akai models. For reliable data entry, use the numeric keypad and +/- keys.

### Panel layouts

Four cosmetic panel views are available via the **Panel Layout** dropdown in the settings area:

1. **Compact** (default)
2. **Full Keyboard**
3. **Rack Panel**
4. **Tablet View**

All views control the same underlying emulation; they differ only in visual layout and scale.

### Window resizing

The plugin window can be resized. It maintains the panel aspect ratio and clamps between 900 px and 2400 px width.

---

## Loading Samples and Media

The S3000XL loads samples from floppy disks, CD-ROMs, and SCSI hard disks. The plugin supports all three via disk image files.

### MEDIA button

Click the **MEDIA** button in the plugin UI to open the media menu. It has three sections:

#### Floppy (hot-swappable)

- **Insert Floppy Image** — Select a floppy disk image file.
- **Eject Floppy** — Removes the currently loaded floppy.
- Supported formats: `.img`, `.hfe`, `.dsk`, `.ima`, `.imd`, `.td0`, `.ipf`
- Floppy images can be inserted and ejected while the machine is running.

#### CD-ROM (reboots on change)

- **Insert CD-ROM Image** — Select a CD-ROM image file.
- **Eject CD-ROM** — Removes the currently loaded CD.
- Supported formats: `.chd`, `.cue`/`.bin`, `.iso`, `.toc`, `.nrg`, `.gdi`
- Inserting or ejecting a CD-ROM reboots the emulated machine so the firmware's SCSI scan can detect the change.

#### SCSI Hard Disk (reboots on change)

- **Set Hard Disk Image** — Select a hard disk image file.
- **Remove Hard Disk** — Detaches the currently loaded hard disk.
- Supported formats: `.chd`, `.hd`, `.hdv`, `.2mg`, `.hdi`
- Changing the hard disk reboots the emulated machine.

### Important notes on media

- **CHD hard disk images are opened exclusively.** If another MAME instance or another plugin instance has the same `.chd` file open, the plugin will fail to boot. Close any other programs using the same disk image.
- **CD-ROM timing:** The plugin uses a realistic 2x CD-ROM transfer speed internally. This is required for clean sample loading. Do not be alarmed by the loading speed; this matches the original hardware behavior.
- Media paths are saved with your DAW project and restored automatically on reload.

---

## MIDI

### MIDI input

The plugin receives MIDI from your DAW's MIDI track and forwards it to the emulated S3000XL. Notes, program changes, control changes, pitch bend, and SysEx messages are all supported.

MIDI timing is sample-accurate: each byte is delivered to the emulation at the correct timestamp relative to the host clock.

### MIDI output

The emulated S3000XL can also send MIDI (e.g., MIDI Thru, SysEx dumps). These are forwarded to the DAW's MIDI output bus and can be routed to other tracks or recorded.

---

## Plugin Settings

### Internal Buffer

A dropdown to adjust the MAME-to-DAW ring-buffer threshold. Available sizes: 128, 256, 512, **1024** (default), 2048, 4096, 8192 samples.

- Smaller values reduce latency but may cause audio dropouts if your system cannot keep up.
- Larger values add latency but improve stability.
- The default of 1024 is a good starting point for most systems.

### Reported latency

The plugin reports approximately 24.4 ms of internal latency to the DAW (e.g., ~1076 samples at 44.1 kHz). Your DAW's delay compensation should handle this automatically.

---

## DAW State and Recall

When you save your DAW project, the plugin persists:

- The emulated machine's internal RAM state (programs, samples in memory)
- All loaded media paths (floppy, CD-ROM, HDD)
- Panel layout selection and window size
- Buffer size setting

When the project is reopened, the plugin reboots the emulation and restores the saved state, including re-mounting any media images that were loaded.

---

## Troubleshooting

### Black panel / no display

The most common cause is a missing `hd61830.bin` LCD character-generator ROM. Ensure both `s3000xl_v2_0.bin` and `hd61830.bin` are present in `Documents\S3000XL\ROMs\s3000xl\`.

### Orange warning screen on load

ROM files are missing or incomplete. Follow the [ROM Setup](#rom-setup) instructions.

### No sound

- Ensure the plugin is on a MIDI track and receiving MIDI input.
- Check that samples are loaded into the emulated machine (use the S3000XL's own Load function via the panel, or mount a disk image with pre-loaded content).
- Verify the **Internal Buffer** setting is not set too low for your system.

### DAW freezes on load

If MAME fails to boot (e.g., a previously loaded hard disk image is no longer available), the plugin clears the bad media path and sets itself to a safe state. If the DAW still freezes:

1. Navigate to `Documents\S3000XL\` and remove or rename any settings files.
2. Reload the project.

### Crackling or corrupted audio when loading samples from CD

This should not happen under normal operation. The plugin uses the correct 2x CD-ROM timing internally. If you experience crackling, ensure you are using the plugin's MEDIA button to load CDs rather than passing command-line overrides.

### Plugin not found in DAW scan

Ensure the `.vst3` bundle is in your DAW's plugin scan path. The standard location is `C:\Program Files\Common Files\VST3\`. Rescan after copying.

### ".7z not supported" warning

The plugin cannot extract `.7z` archives. Use 7-Zip or a similar tool to extract the contents first, then point the ROM locator to the extracted folder or `.zip` file.

---

## Legal

This project is distributed under **GPLv3**. It is **not affiliated with, endorsed by, or sponsored by** any hardware manufacturer. Product and company names are trademarks of their respective owners and are used only to identify the hardware being emulated.

No ROMs, firmware, or disk images are included. The plugin is non-functional without user-supplied firmware.
