---
type: Architecture
title: Input Handling — Panel Buttons, DATA ENTRY Dial, and MIDI
description: How user input from the JUCE editor and the DAW is translated into MAME ioport state, including the 36-button clickmap, the analog DATA ENTRY dial, and timestamped MIDI.
resource: ../plugin/Source/PluginProcessor.cpp
tags: [input, buttons, dial, midi, ioport, mame, clickmap]
generated:
  by: agent/devin
  at: 2026-09-09T21:30:00Z
verified: []
status: stable
stale_after: 2027-03-09
sources:
  - id: source-processor
    resource: ../../plugin/Source/PluginProcessor.cpp
    title: PluginProcessor.cpp — set_value and MIDI logic
    author: agent/devin
    last_modified: 2026-09-09
  - id: source-processor-h
    resource: ../../plugin/Source/PluginProcessor.h
    title: PluginProcessor.h — sd1Buttons table and params
    author: agent/devin
    last_modified: 2026-09-09
  - id: source-editor
    resource: ../../plugin/Source/PluginEditor.cpp
    title: PluginEditor.cpp — hit test and mouse handlers
    author: agent/devin
    last_modified: 2026-09-09
  - id: port-map
    resource: ../../S3000XL-PORT-MAP.md
    title: S3000XL port map
    author: human:steve
    last_modified: 2026-07-26
---

# Input Handling — Panel Buttons, DATA ENTRY Dial, and MIDI

User interaction flows through three paths: mouse clicks on the rendered panel, the DATA ENTRY dial, and MIDI messages from the DAW.

## Panel button matrix

The MAME driver exposes eight ioport groups `C0..C7`. Each group is active-low and uses five bits (`0x01`, `0x02`, `0x04`, `0x08`, `0x10`). The plugin defines a single `sd1Buttons` table mapping each logical button to an ioport tag and mask:

- 8 mode buttons (`Load`, `Save`, `Global`, `Edit`, `Effects`, `Sample`, `Multi`, `Single`) — mask `0x01`
- 8 soft keys (`F1..F8`) — mask `0x02`
- 10 numeric/edit keys, cursor arrows, `Enter/Play`, etc. — masks `0x04`, `0x08`, `0x10`
- 2 DATA ENTRY click zones (`btn_dial_inc`, `btn_dial_dec`) on the `:DATAENTRY` analog port

Total: **38 boolean VST parameters** + the analog `data_entry` parameter.

### Click-to-ioport path

1. `PluginEditor::mouseDown` performs `mpcHitTest(px, py)` against the generated clickmap (`mpc_clickmap.h`).
2. A hit returns a button index into `sd1Buttons`.
3. `parameterChanged` writes the button's `ioport_field` value directly via `set_value(1.0)` (down) or `set_value(0.0)` (up).
4. MAME's polled matrix sees the active-low bit briefly asserted.

> MAME's mouse-pointer injection does **not** reliably drive a polled keyboard matrix. `set_value` on the field is the reliable path.

## DATA ENTRY dial

The physical dial is `IPT_DIAL` in MAME and produces an absolute 8-bit position (0..255). The firmware derives rotation from changes in that value, so the plugin must:

- Maintain an atomic `dialValue` (0..255).
- Map the `data_entry` VST parameter (0..1) to 0..255.
- Handle click zones (`btn_dial_inc`/`btn_dial_dec`) as ±1 wraps.
- Handle mouse drag-to-turn by scaling vertical drag pixels into position changes.

For S3000XL, the dial is also intentionally made **wrapping** (`& 0xFF` instead of `jlimit(0,255,…)`), matching the hardware: turning past 255 wraps to 0 and vice versa.

## MIDI input

Incoming DAW MIDI is queued with a target MAME time so MAME reads bytes at the correct emulation timestamp:

- `pushMidiByte(data, targetMameTime)` writes into a lock-free ring buffer (`midiBuffer`).
- `VstMidiInputPort::read()` returns a byte only when `mameMachine->time() >= targetMameTime`.
- This avoids the old "drift" behavior and makes MIDI timing sample-accurate relative to the host clock.

## MIDI output

`VstMidiOutputPort` pushes bytes from the emulated DUART TX into a separate ring buffer. `processBlock` assembles running-status messages and forwards them to the DAW's MIDI output bus.

## Parameter layout

See `PluginProcessor::createParameterLayout()` for the full list. Important identifiers:

- `data_entry` — float 0..1 mapped to dial absolute position
- `btn_load`, `btn_save`, `btn_global`, `btn_edit`, `btn_effects`, `btn_sample`, `btn_multi`, `btn_single`
- `btn_f1`..`btn_f8`
- `btn_key0`..`btn_key9`, `btn_plus`, `btn_minus`, `btn_enter`, `btn_name`, `btn_mark`, `btn_jump`
- `btn_up`, `btn_down`, `btn_left`, `btn_right`
- `buffer_size` — MAME ring-buffer threshold combo
- `layout_view` — panel layout choice

## Save program macro

The plugin implements a two-phase save macro:

1. User clicks the save badge → plugin sends a virtual `WRITE` button press and arms detection.
2. User clicks a numeric bank button on the panel (`0..9`) → the audio thread holds that bank via `macroBankToHold`.
3. User clicks a soft key (`F1..F8`) to choose the destination slot.

This mirrors the hardware workflow because there is no discrete "save to slot N" command exposed by the firmware.
