# S3000XL-VST — Port map (A2A)

Target: **S3000XL-VST** (brand-neutral name; on-screen panel shows "AKAI S3000XL" = MAME artwork, nominative).
Source of truth: `F:\MPC3000vsti\References\mame\src\mame\akai\s3000.cpp` (MAME 0.288, driver by R. Belmont, BSD-3).
Machine short name: **`s3000xl`**  (`SYST( 1994, s3000xl, 0, 0, s3000xl, s3000xl, s3000_state, empty_init, "Akai", "S3000XL", MACHINE_NOT_WORKING)` @ L1068).

## Why this is a light port (vs MPC3000)
Header (L4-12): *"these rackmount samplers are all built on variants of what became the MPC3000 hardware."*
Same core devices as our MPC3K-VST plugin already drives — **no 7810/7811 sub-CPU, no drum-pad scan matrix** (simpler input).

| Subsystem | Device (s3000.cpp) | Reuse from MPC3K-VST |
|---|---|---|
| CPU | `v53a_device` (L161) | yes |
| Sound DSP | `l7a1045_sound_device` -> `SPEAKER(2)` L6028_LEFT/RIGHT (L681-688) | audio sink tap = verbatim (machine-agnostic) |
| LCD | `HD61830` / LC7981 (L630) | render->screenBuffers verbatim; **needs `hd61830.bin` char-gen device ROM (already have it)** |
| FDC | `upd72069_device` (L167) | media manager verbatim |
| SCSI | MB89352 (header L23) + CD/HDD | media manager verbatim (CD/HDD load WORKS per header) |
| MIDI | `midi_port_device` (L166) | verbatim |
| Panel input | 8x ioport `C0..C7` + `DATAENTRY` dial | hit-test -> buttonParams (set_value) |
| Layout | internal `layout_s3000xl` -> `s3000xl.lay` (L735) | GUI comes free from MAME artwork |

## ROM manifest (s3000xl, default BIOS "ver 2.00", ROM_START @ L1033)
- `s3000xl_v2_0.bin` — 0x80000 (512 KB), single file, CRC 26a00ddd, SHA1 ace009953aeccff4dfbee0ccfa091a0fea39a386. **maincpu region** (no LSB/MSB split — unlike base s3000).
- Device ROM (from HD61830 device): `hd61830.bin` (1472 bytes) — **the black-panel gate; we already have this from MPC3000.**
- Alt BIOSes available: v1.50 (`akai s3000xl v1-50.bin`), v1.06 (`akai s3000xl v1-06.bin`).
- ROM folder (plugin convention): `Documents/S3000XL/ROMs/s3000xl/`.

## Panel button matrix (INPUT_PORTS_START(s3000xl) L838-901)
Port groups C0..C7, active-low, bits 0x01/0x02/0x04/0x08/0x10:
- C0: Load, F8, "1", "0"
- C1: Save, F7, "2", "-", Right-Arrow
- C2: Global, F6, Mark, Jump, Up-Arrow
- C3: Edit, F5, "9", "6"
- C4: Effects, F4, "8", "5"
- C5: Sample, F3, "7", "4"
- C6: Multi, F2, Name, Enter/Play, Down-Arrow
- C7: Single, F1, "3", "+", Left-Arrow
- DATAENTRY: `IPT_DIAL` 0xff (data-entry knob; same as MPC3000 -> +/- or dial control)

## NOT emulated (known, for later)
- **Recording / sampling audio-in**: header TODO "Recording." (L51). A/D is only an i8255 register stub `m_wadcs` "WADCS = A/D converter" (L171, L305, L317). Real analog->sample path NOT modelled. Deferred to a separate feasibility spike (would be genuine MAME device-emulation work).
- **FX**: EB16 effects DSP (L7A1414-L6038) is an optional card, not in the base machine config -> no onboard effects (use DAW FX).

## Build recipe (same as MPC3K-VST)
1. driver_list override -> `driver_s3000xl`; argv `s3000xl`.
2. ROM manifest -> s3000xl set + folder; keep `hd61830.bin`.
3. `s3000xl.lay` -> regenerate hit-test clickmap (bounding box TBD from the .lay).
4. Button ioport tags C0..C7 masks (above) -> buttonParams.
5. MAME libs: genie `SUBTARGET=s3000xl SOURCES=src/mame/akai/s3000.cpp`; MSBuild (toolset 14.44, x64 host).
6. .jucer -> mame_s3000xl.lib, name/pluginName/targetName "S3000XL-VST"; Projucer resave; build VST3.
7. Standalone gate FIRST: `mame s3000xl -verifyroms` = best available, boots to LCD.

## User requirements / notes (2026-07-26)
1. **DATAENTRY dial** — known MAME-side bug on ALL Akai models (dial behaves incorrectly).
   Not ours to fix. Reliable data entry = the +/- keys and numeric keypad (C0..C7),
   like MPC3000. Keep the dial mapped but do not rely on it.
2. **Maximum RAM build** — requirement: ship the machine with max sample RAM.
   Evidence: the sample/DSP space is mapped `map(0x0000'0000, 0x01ff'ffff).ram()` (L364)
   = **32 MiB**, already the maximum, and `.ram()` auto-allocates the whole region -> the
   emulated machine physically HAS 32 MiB. The catch is firmware RAM-sizing: the SIMM-ID
   detection routine (L404-425, PC=0x20ed9 in S3000 v2.0) probes SIMM type; comment warns
   *"S3000 only recognizes a total of 8 MiW (16 MiB)... memory test can't handle [32 MiB]"* (L424-425).
   ACTION at build/runtime: confirm what S3000XL **v2.0** firmware actually sizes; if the
   driver fixes/reads a SIMM-ID, set it to the largest size the XL reliably tests (target 32 MiB,
   fall back to 16 MiB if the XL memory test also chokes). Verify empirically in the LCD "GLOBAL".
3. **Multi-instance** — requirement: several plugin instances per project (sampler use).
   RISK/CAVEAT (honest): MAME uses process-global/static state; the 0.288 MPC3K-VST build
   deliberately went single-instance for that reason. True multi-instance in one DAW process
   needs either (a) isolating/duplicating MAME globals, or (b) out-of-process hosting.
   The Sojus base isolates per-instance DATA dirs (temp + NVRAM lock + Uuid), which helps with
   files but NOT with in-process global singletons. -> Treat multi-instance as its own scoped
   task with a feasibility check BEFORE promising it; do not assume it "just works" on 0.288.
