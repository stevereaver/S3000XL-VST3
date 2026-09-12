# MAME → VSTi Conversion Recipe (Sojus-pattern)

Generalized, battle-tested recipe for wrapping a **MAME machine** as a **VST3/AU
instrument**, built on the Sojus "Ensoniq SD-1" headless-MAME host pattern and refined
through two full ports: **Akai MPC3000** (`MPC3K-VST`) and **Akai S3000XL** (`S3000XL-VST`).

> A2A discipline: the source/repo is the judge. Grep before you claim a symbol exists.
> The compiler and the DAW are the final judges. Verify standalone before wrapping.

---

## 0. Standalone gate FIRST (do this before writing any plugin code)

1. `mame <sys> -verifyroms <sys>` → must reach **"best available"** (the machine's own
   CPU/panel ROMs present). Missing **device ROMs** count too — see §2.
2. `mame <sys>` boots to its screen and (ideally) makes sound. `MACHINE_NOT_WORKING` is
   usually **administrative** (imperfect save/sound), not "does not run" — evidence wins.
3. `mame <sys> -wavwrite t.wav -seconds_to_run 6` → **read the WAV header** to learn the
   machine's true output channel count (this is the audio stride, see §3 — the single most
   important machine-specific number).

Only after all three pass do you wrap it.

## 1. Pick & verify the driver

- Locate `src/mame/<vendor>/<file>.cpp`. Confirm the `SYST(...)` / short name.
- List the core devices (CPU, sound DSP, LCD, FDC, SCSI, MIDI). If they match a machine you
  already wrapped, ~90% of the host code is reusable (copy the working plugin as the base).
- Note whether there's a **sub-CPU-polled key matrix** (drum pads) or just a simple key
  matrix — the latter is simpler (no low-latency sub-CPU).

## 2. Core swap (from a working reference plugin)

- `driver_list::s_drivers_sorted[]` override → `&driver_<sys>`; argv `push_back("<sys>")`.
- ROM manifest → the machine's ROM set + folder `Documents/<AppName>/ROMs/<sys>/`.
- **Device ROMs are a silent trap — the "black panel":** the LCD character-generator ROM
  (e.g. **`hd61830.bin`**, an LC7981/HD61830 char-gen, often flagged NO_DUMP/NEEDS REDUMP)
  must be present, or MAME fails to start the machine **silently** → empty render list →
  **black panel**. Add device ROMs to the plugin's self-check so a missing one gives a clear
  message instead of a black screen.
- **Buttons:** the driver's `INPUT_PORTS` (`IPT_KEYBOARD` scan matrix, e.g. tags `C0..C7`)
  → layout **hit-test → buttonParams → `ioport_field::set_value()`**. MAME's mouse-pointer
  injection does NOT drive the polled matrix; `set_value` is the reliable path.
- **Auto-generate the click-map from the machine's `.lay`**: every clickable element carries
  `inputtag`/`inputmask`/`<bounds>`. Parse them, apply the **group transform** for elements
  inside `<group>`s (group-local coords → view coords via the group's placement bounds), and
  match `(inputtag,inputmask)` to the button-table index. Compute `VIEW_W/VIEW_H` from the
  view's bounding box (drives the letterbox + window aspect).

## 3. AUDIO OUTPUT — the critical, machine-specific bit ⭐ (hard-won)

MAME delivers the final speaker mix to the OSD as **interleaved int16**, with
**STRIDE = `m_outputs_count`** = the machine's total speaker output channels. This is **NOT a
constant** — it differs per machine:

| Machine  | Output channels (stride) | Channel layout                                   |
|----------|--------------------------|--------------------------------------------------|
| SD-1     | 5                        | Main L, Main R, Aux L, Aux R, Floppy             |
| MPC3000  | 11                       | Main L/R + 8 individual outs + …                 |
| S3000XL  | 3 with floppy device     | **ch0=floppy, ch1=L, ch2=R**                      |

- **Measure the stride**: the `-wavwrite` WAV header's channel count = `m_outputs_count`.
  Confirmed in `src/emu/sound.cpp`: `wav_add_data_16(wav, buf, samples * m_outputs_count)`.
- **Wrong stride = corrupt render**: too big → periodic **silent gaps** (over-reads a short
  buffer); too small → **scrambled / mis-read** audio. Not affected by sample rate or buffer
  size → it looks like a "deterministic render bug." A sustained sine test tone exposes it.
- **Channel ORDER matters**: identify main L/R among all speakers, including nested devices.
  S3000XL's configured floppy adds a speaker before the main stereo speaker. The observed
  3-channel layout uses `pcm[i*3+1]` / `pcm[i*3+2]` for main L/R. The plugin queries
  `sound().outputs_count()` for the stride and uses channels 0/1 in the 2-channel case.
- **Producer buffering must cover a host block**: use at least the host block size as the
  producer's buffering threshold, even when the Internal Buffer setting is smaller. Otherwise
  every host callback can run short and insert silence. MIDI timing, prefill and PDC must use
  the same effective threshold.
- The tap itself is machine-agnostic: `osd::add_audio_to_recording()` at the host rate
  (pass `-samplerate <hostRate>`). Only the **stride + L/R mapping** is per-machine.

## 4. SCSI media (hard-won)

- **Driver SCSI defaults differ** — check the `NSCSI_CONNECTOR(...)` lines:
  - MPC3000: all slots `nullptr` (empty) → the plugin ADDS `-scsi:N cdrom`/`harddisk`.
  - S3000XL: `scsi:4` defaults to **`cdrom_2x`**, `scsi:5` to **`harddisk`** (pre-populated).
  Don't blindly override a pre-populated slot; prefer the driver default + just load the image.
- **CD-ROM device timing = DATA INTEGRITY** ⭐: `cdrom` and `cdrom_2x` share the SAME SCSI
  identity ("Sony CDU-76S") and differ ONLY in **data timing** (`cdrom_2x` overrides
  `scsi_data_byte_period` = 307200 B/s + a 100µs read command delay). The firmware needs the
  **paced 2x transfer** to read a disc **cleanly**; plain `cdrom` transfers too fast →
  **corrupted sample data → crackly loads**. (HDD & `cdrom_2x` load clean; plain `cdrom`
  crackles.) `cdrom_2x` does **not** slow the emulation (measured 186% with/without a disc).
  → **Use `cdrom_2x` when a disc is loaded**, plain `cdrom` when empty (an idle 2x drive can
  stall the firmware main loop / live MIDI with spin-up retries).

## 5. Media file locking & boot robustness (hard-won)

- MAME opens a **harddisk `.chd` READ-WRITE EXCLUSIVE**. If a **second MAME instance**
  (e.g. standalone MAMEUI) holds the same disk, the plugin's open fails with
  **`permission denied (generic:13)`** → MAME fatal-errors. Don't run standalone MAME on the
  same disk while testing the plugin.
- **Graceful boot-failure (mandatory):** capture the return code of
  `cli_frontend::execute(args)`. On non-zero:
  1. `isMameRunning = false` — else the audio thread waits forever for a dead engine → the
     **whole DAW freezes**.
  2. Clear the persisted media paths (`scsi_hdd_path`/`scsi_cd_path`) and save — else the bad
     path is reloaded next launch → **crash-loop that bricks the plugin**.

## 6. Unique plugin identity ⭐ (hard-won)

Each machine's plugin needs a **UNIQUE 4-char `pluginCode`** in the `.jucer`. Two JUCE plugins
that share a `pluginCode` (e.g. both `Mp30`) generate the **same VST3 UID** → the DAW confuses
their **parameter caches** and shows only one plugin's params (symptom: "only Volume appears").
- MPC3000 = `Mp30`, S3000XL = `S3kx`. Also keep `pluginName`, `bundleIdentifier`, and the AU
  export prefix unique.

## 7. Data-entry dial (analog)

- The Akai data-entry knob is `IPT_DIAL` (analog), not a digital button. Drive it via
  `field->live().analog->set_value(0..255)` — NOT digital `set_value`. The firmware reads the
  absolute value and derives rotation deltas. Wire it to: click zones (±1), drag-to-turn, and
  a `data_entry` VST float param (0..1 → 0..255). (Note: the dial has a known MAME-side quirk
  on all Akai models.)

## 8. Render / LCD pump — OPEN ITEM (WIP)

- The OSD `update(bool skip_redraw)` grabs the MAME frame into `screenBuffers[]`. It
  early-returned on `skip_redraw`; under `-nothrottle` MAME frameskips heavily → the grab is
  skipped → the panel only refreshes when an **event** forces a rendered frame.
- Decoupling the grab from `skip_redraw` was attempted but did **not** fully fix it — still
  under investigation. Candidate: pump the grab on a fixed cadence (or drive
  `machine().video().frame_update()` / render from the editor timer) independent of MAME's
  frameskip decision.

## 9. Build toolchain (quick reference)

- genie: `make vs2022 SUBTARGET=<sys> SOURCES=src/mame/<vendor>/<file>.cpp NOWERROR=1`
  (`OS=Windows_NT`, MSYS2 MINGW64). MAME lib rebuild needs the `_CL_` NTDDI override.
- MSVC: `vcvarsall x64 14.44.35207`, `/p:VCToolsVersion=14.44.35207
  /p:PreferredToolArchitecture=x64` (sol.hpp/big PCH exhaust the 32-bit host).
- **Plugin build: NO `_CL_`/NTDDI override** (it breaks JUCE harfbuzz/DirectWrite). Same
  toolset as the libs (C++ ABI).
- Libs land in `build/vs2022/bin/x64/Release/` (shared: emu, cpu, …) **and** a per-subtarget
  subdir `.../Release/mame_<sys>/` (mame_<sys>.lib, optional.lib, dasm.lib, formats.lib) →
  `libraryPath` must include **both**. `.jucer`: `mame_<sys>.lib`; drop `sd1disk.lib` (SD-1
  only); `softfloat3.lib` only.
- **Transient `LNK1105` (error 1224)** on the shared `S3000XL-VST.lib`: a lingering
  linker/PDB process has it mapped. **Do NOT kill processes mid-build** (orphans file handles
  and makes it worse); let the current build finish, then retry cleanly. `mspdbsrv.exe`
  lingering between builds is normal. The DAW holding the old `.vst3` also blocks the
  post-build copy — reload/close the plugin, or copy the DLL into the bundle manually.

## 10. Legal

- Ship **NO ROMs / no disk images** (the plugin is non-functional without them = legal to
  distribute). Brand-neutral repo name (`S3000XL-VST`, not "Akai …"), a NOT-affiliated
  trademark disclaimer, and **GPLv3** (MAME is GPL-2.0-or-later, JUCE under its GPLv3 terms).
  The on-screen panel showing "AKAI S3000XL" is MAME's own hardware artwork (nominative use).
