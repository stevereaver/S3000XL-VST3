---
type: Architecture
title: Audio Pipeline — MAME to VST3 Output
description: How MAME's multi-channel interleaved output is mapped to the plugin's stereo VST3 output, including stride, channel selection, and throttling.
resource: ../plugin/Source/PluginProcessor.cpp
tags: [audio, mame, ring-buffer, stride, stereo, output]
generated:
  by: agent/devin
  at: 2026-09-09T21:30:00Z
verified: []
status: stable
stale_after: 2027-03-09
sources:
  - id: source-processor
    resource: ../../plugin/Source/PluginProcessor.cpp
    title: PluginProcessor.cpp
    author: agent/devin
    last_modified: 2026-09-09
  - id: source-processor-h
    resource: ../../plugin/Source/PluginProcessor.h
    title: PluginProcessor.h
    author: agent/devin
    last_modified: 2026-09-09
  - id: recipe
    resource: ../../MAME-to-VSTi-RECIPE.md
    title: MAME → VSTi Conversion Recipe
    author: human:steve
    last_modified: 2026-07-26
---

# Audio Pipeline — MAME to VST3 Output

MAME renders the `s3000xl` machine's final speaker mix as interleaved `int16` samples and hands it to the plugin's headless OSD callback. The plugin converts it to floating point and writes it into ring buffers consumed by `processBlock`.

## Key numbers

| Value | S3000XL | Where it lives |
|-------|---------|----------------|
| `m_outputs_count` (stride) | **3** with the configured floppy device | Queried at runtime via `sound().outputs_count()`; includes nested device speakers |
| Main L | channel 1 (3-ch), channel 0 (2-ch) | `ringBufferL` |
| Main R | channel 2 (3-ch), channel 1 (2-ch) | `ringBufferR` |
| Extra channel | floppy speaker at channel 0 (3-ch) | intentionally dropped |

## Data flow

1. MAME calls `osd::add_audio_to_recording()` with `samples` frames containing `samples * m_outputs_count` interleaved `int16` values.
2. `pushAudioFromMame(pcmBuffer, numSamples)` receives `numSamples` MAME audio frames.
3. At runtime it queries `mameMachine->sound().outputs_count()` to get the interleaved stride.
4. For each frame `i` it reads the main L/R channels (skipping any extra mechanical/disk channel) and converts to float.
5. `processBlock` reads from the ring buffers at the host sample rate.

## Audio-driven throttling

MAME runs on a dedicated thread and can outpace the DAW. `pushAudioFromMame` blocks when the ring-buffer distance (`totalWritten - totalRead`) exceeds a threshold:

- Real-time: `getEffectiveBufferThreshold()` is the maximum of the Internal Buffer setting (default 512) and the host block size supplied to `prepareToPlay`. The producer must be allowed to fill at least one complete host block before sleeping.
- Offline/non-realtime: `maxOfflineBuffer`, set to the current block size plus the effective threshold while rendering.
- MIDI scheduling, prefill, and latency reporting use the same effective threshold. Reported latency is this threshold plus `getInternalHardwareLatencySamples()`.

A user recording at 44.1 kHz with 2048-frame host blocks contained 882-frame audio chunks followed by 1166-frame silence gaps. The old 512-frame producer threshold stopped after one MAME chunk; `processBlock` padded the shortage with zeros. Matching sample rates do not exclude this defect. The host-block minimum fixes this deterministic shortage; actual DAW playback still requires verification.

On Windows the wake interval is 1 ms; on macOS it remains 5 ms. This reduces scheduling delay but cannot guarantee underrun-free playback.

## Internal hardware latency

`getInternalHardwareLatencySamples()` returns `0.0244 * hostSampleRate`, e.g. ~1076 samples at 44.1 kHz. This is reported to the DAW as plugin latency.

## Common mistakes

- Wrong stride: mismatching `m_outputs_count` scrambles audio or inserts silent gaps. The plugin now queries `sound().outputs_count()` at runtime to avoid hard-coding the stride.
- Channel order: identify main L/R among the channels. For S3000XL the main stereo is the first two speaker channels (or channels 1/2 if a mechanical channel is present).
- Mixing float/int scaling: MAME samples are `int16`; divide by `32768.0f`.
- **Missing wave RAM on state restore**: the plugin saves OS RAM and Seq RAM on `getStateInformation` and injects them on warm boot via `pendingRamInjection`. If the DSP wave RAM (the L7A1045's 32 MiB sample memory at `s3000.cpp:dsp_map`) is NOT saved and restored alongside them, the firmware believes samples are loaded (OS RAM state intact) but the actual sample data is missing from the DSP's wave memory, so the DSP reads garbage and the panel PLAY button produces distortion/buzz. The fix exposes wave RAM as the named share `"waveram"` in `s3000.cpp:dsp_map`, saves it zlib-compressed under `ram_waveram` in `getStateInformation`, and injects it in the same `pendingRamInjection` path as `osram`/`seqram`.
