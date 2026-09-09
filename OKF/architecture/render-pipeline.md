---
type: Architecture
title: Render Pipeline — LCD and Panel Display
description: How the emulated S3000XL panel is rendered into the JUCE editor window, including MAME layout, double-buffered video, and the LCD live-refresh fix.
resource: ../plugin/Source/PluginProcessor.cpp
tags: [render, lcd, mame, layout, panel, texture, double-buffer]
generated:
  by: agent/devin
  at: 2026-09-09T21:30:00Z
verified: []
status: stable
stale_after: 2027-03-09
sources:
  - id: source-processor
    resource: ../../plugin/Source/PluginProcessor.cpp
    title: PluginProcessor.cpp — VstOsdInterface::update
    author: agent/devin
    last_modified: 2026-09-09
  - id: source-editor
    resource: ../../plugin/Source/PluginEditor.cpp
    title: PluginEditor.cpp — paint and timerCallback
    author: agent/devin
    last_modified: 2026-09-09
  - id: state
    resource: ../../STATE.json
    title: Project state file
    author: human:steve
    last_modified: 2026-07-26
---

# Render Pipeline — LCD and Panel Display

The plugin does not draw the panel with custom vector graphics. It allocates a MAME render target, copies the framebuffer into a `juce::Image`, and paints that image into the editor.

## Components

| Component | Role |
|-----------|------|
| `VstOsdInterface` | Headless OSD bridge that owns the MAME render target and frame grab |
| `screenBuffers[2]` | Double-buffered `juce::Image` pool (2560×2560 ARGB) |
| `readyBufferIndex` | Atomic index (0 or 1) of the buffer the UI should paint |
| `PluginEditor::paint` | Scales and draws the ready buffer to the component bounds |
| 30 Hz timer | Polls for new frames and layout changes |

## Frame grab

In `VstOsdInterface::update()`:

1. `update()` is **not** early-returned on `skip_redraw`; MAME often skips frames under `-nothrottle`, and gating the grab on that flag caused the LCD to update only when an input event forced a render.
2. A dirty-frame hash is computed from the render target's texture. The fix adds `prim->texture.seqid` (content change) in addition to `prim->texture.base` (bitmap address), so the LCD refreshes when the emulated screen content changes.
3. If the hash changed, the current MAME frame is copied into the back `screenBuffer`, and `readyBufferIndex` is flipped.

## Layout and view model

- The S3000XL MAME layout defines a single view (`mameViewMap[4] = {0,0,0,0}`).
- The plugin exposes four cosmetic panel layouts: Compact (0), Full Keyboard (1), Rack Panel (2), Tablet (3). All map to MAME view 0.
- Optional debug rack mode (`SD1_DEBUG_RACK_PANEL`) maps view index 4 to MAME view 2.

## Window sizing

`PluginEditor` maintains the panel aspect ratio from `MPC_VIEW_W / MPC_VIEW_H` (defined in `mpc_clickmap.h`) and clamps the window between 900 px and 2400 px width. Resizes propagate back to the processor as atomic `windowWidth`/`windowHeight`, which the OSD applies on the next `update()` via `requestRenderResize`.

## Known issues and open items

- **LCD live refresh:** the `skip_redraw` decoupling and `seqid` hash mostly fixed the "only refreshes on events" problem, but some residual stalls remain under heavy frameskip. A future fix may pump the frame grab on a fixed cadence independent of MAME's frameskip.
- **Black panel:** if the `hd61830.bin` LCD character-generator device ROM is missing, MAME silently fails to start, leaving the render list empty. The plugin's self-check detects this and shows a ROM locator UI instead of a black panel.
