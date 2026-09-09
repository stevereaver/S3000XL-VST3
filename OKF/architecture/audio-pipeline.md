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
| `m_outputs_count` (stride) | **3** | `src/emu/sound.cpp` WAV write path; verified via `-wavwrite` |
| Channel 0 | dropped | floppy/mechanical seek sound |
| Channel 1 | main left | `ringBufferL` |
| Channel 2 | main right | `ringBufferR` |

## Data flow

1. MAME calls `osd::add_audio_to_recording()` with `samples * m_outputs_count` `int16` frames.
2. `pushAudioFromMame(pcmBuffer, numSamples)` receives `numSamples` MAME audio frames.
3. For each frame `i`:
   - `ringBufferL[index] = pcmBuffer[i * 3 + 1] / 32768.0f`
   - `ringBufferR[index] = pcmBuffer[i * 3 + 2] / 32768.0f`
   - Channel 0 is intentionally dropped (no disk rattle in instrument output).
4. `processBlock` reads from the ring buffers at the host sample rate.

## Audio-driven throttling

MAME runs on a dedicated thread and can outpace the DAW. `pushAudioFromMame` blocks when the ring-buffer distance (`totalWritten - totalRead`) exceeds a threshold:

- Real-time: `mameBufferThreshold` (default 1024 samples, configurable via UI combo).
- Offline/non-realtime: `maxOfflineBuffer` (default 1024).

On Windows the wake interval is 1 ms; on macOS it remains 5 ms. This prevents underruns caused by the default scheduler resolution.

## Internal hardware latency

`getInternalHardwareLatencySamples()` returns `0.0244 * hostSampleRate`, e.g. ~1076 samples at 44.1 kHz. This is reported to the DAW as plugin latency.

## Common mistakes

- Wrong stride: using 2 instead of 3 scrambles audio or inserts gaps. Always derive from `-wavwrite` / `m_outputs_count` for a new machine.
- Mapping channel 0 to the main out: it carries the floppy seek sound.
- Mixing float/int scaling: MAME samples are `int16`; divide by `32768.0f`.
