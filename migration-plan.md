# Migration Plan — Synthesizing the MIDI roll through the audio pipeline

## Purpose

Render the same MIDI roll we render today, but through the **audio-native graph core** instead of
the procedural `Synthesizer`/`AudioChannel`/`AudioNote` path — so that sampling the synth becomes
"set the ambient `AudioContext`, render the graph, read the output," and so we can then compose new
node types to change the sound. The procedural path stays as a reference oracle until the pipeline
path reaches parity.

**Out of scope:** a pipeline DSL (we wire graphs manually in C++), real-time device output, and
block sizes > 1. Those are deferred; nothing here precludes them.

## What already exists (the core — do not rebuild)

The core landed on branch `audio-pipeline-bridge` under `include/Audio/Processing/Graph/`:

- `Signal` = `double` — one **mono** channel per edge. Stereo = two ports/edges; Pan is 1-in→2-out.
- `AudioContext` — the **single** object threaded to every node: ambient read-only fields
  (`sampleRate`, `transportSample`, `noteSample`, `frequency`, `gate`, `velocity`, `volume`, `pan`,
  `blockSize`) plus `input(i)` / `output(o)` / `state()` accessors bound during the sweep.
- `SignalProcessor` — a pure, reusable kernel: `void process(AudioContext&) const`, declaring
  `inputCount()`, `outputCount()`, `stateSize()` (doubles of per-instance state), `resetState()`.
- `SignalGraph` — pure-data description (`add`, `connect`, `connectDelayed`, `setOutput`) →
  `compile(maxInstances, blockSize)`.
- `AudioPipeline` — the compiled graph: flat topo-ordered node table + scratch arena + pooled
  per-instance state arena; `renderInstance(i, ctx)` is a straight linear sweep; `resetInstance(i)`.

**Key terms:** the pipeline renders an **instance** — one running copy of a compiled graph with its
own state slice. A musical **voice** (one sounding note) is *realized as one instance*; the "voice"
concept lives in the instrument layer, not the core.

Per-instance state is reached only via `ctx.state()` and is addressed by compile-time integer
offsets into the arena (the real-time-safe "resource map by id"). This is the mechanism that lets
one compiled graph serve many simultaneous notes.

## Guiding principles (already true in the core; keep them true in the DSP layer)

1. **Processors are stateless.** Config (waveform, gain, envelope times, delay length) lives as
   members; all per-instance mutable state (phase, envelope position, delay buffer) lives in
   `ctx.state()`. This is what makes one voice graph serve N notes.
2. **One `AudioContext`.** Everything a node needs — ambient data, I/O, and state — comes through it.
3. **Mono `double` edges.** The voice chain stays mono until Pan; stereo is two edges.
4. **Block-ready, `blockSize == 1`.** Nodes loop `for f in 0..ctx.blockSize`; time within a block is
   `ctx.noteSample + f`. No signatures change when real blocks arrive.
5. **Reproduce before improve.** Keep `synthesizeMidiRoll()` (procedural) as the oracle until the
   pipeline path matches it on `moonlight-sonata.json`.

## How the pieces map onto the core

The synth is **not** one giant graph. It is a small hierarchy of compiled graphs plus summation,
because instances are rendered in isolation and only meet when summed:

- **Voice graph** — a `SignalGraph` = *synthesis stage → dynamics stage*, compiled **once** into an
  `AudioPipeline`. Each active note is **one instance** of it.
- **Instrument** — owns that one voice `AudioPipeline`, an instance pool (= `maxInstances` =
  polyphony), the active-note bookkeeping, an `InstrumentConfig`, and the note API. `sample()`
  loops active instances: set the instance's ambient context (`frequency`, `gate`, `velocity`,
  `noteSample`), `renderInstance(i, ctx)`, and **sum** the outputs. This is the shared-pipeline /
  per-instance-state model made concrete.
- **Channel** — owns an `Instrument` + an optional **post-FX** `AudioPipeline` (compiled with
  `maxInstances == 1`, rendered once on the *summed* signal) + channel `volume`/`pan`, and produces
  the stereo `AudioSample`. (Nice consequence of the core: post-FX being a 1-instance pipeline means
  its reverb/delay state is naturally single/shared — one reverb per channel — while a delay placed
  *inside* the voice graph is automatically per-note.)
- **Synthesizer** — sums channels (as today), keeps implementing the `MidiEventProcessor` visitor
  unchanged; `channelOn` creates a channel, `keyOn`/`keyOff` route to
  `channel → instrument.playNote/releaseNote` with **velocity passed through** (fixes the
  `// Should send the strength as well` stub in `Synthesizer.cpp`). `sample()` advances
  `transportSample` and drives one tick.

### Synthesis stage (the instrument's sound)
The **synthesis subgraph** produces the instrument's raw timbre *before* dynamics. Its makeup is its
own concern — **not** the instrument's or `InstrumentConfig`'s: it may wire **zero, one, or many**
`OscillatorNode`s (and/or noise/sample sources), and **waveform is a property of each oscillator**,
never of the instrument. Synthesis-internal settings live here: waveform per oscillator, oscillator
count, unison/detune, oscillator mix, harmonic count. The voice graph is therefore:
**synthesis subgraph → `ADSRNode → VCA` (dynamics, × velocity) → output.**

### `InstrumentConfig` (suggested settings)
Instrument-level concerns only — explicitly **not** how the sound is synthesised:
- **Envelope**: attack, decay, sustain level, release (seconds).
- **Amplitude**: master gain, velocity sensitivity.
- **Pitch**: playable frequency range (min/max), glide/portamento time.
- **Polyphony**: `maxInstances`, voice-stealing policy (steal oldest) when the pool is exhausted.
- **Post-FX**: enable/params where the instrument (or channel) owns its own post chain.

## The `Nodes` folder

Create `include/Audio/Processing/Nodes/` (+ `src/...` where a `.cpp` is warranted) and add it to
`include_directories` in `CMakeLists.txt`. Each node is a `SignalProcessor`; per-instance state is
declared via `stateSize()` and read/written through `ctx.state()`:

| Node | From | State (`ctx.state()`) | Notes |
|---|---|---|---|
| `OscillatorNode` | `Oscillator` + `IWaveform`/`SineWave`/`SawtoothWave` | phase (1 double) | 0-in/1-out. Emits from `ctx.frequency`. **Keep the `t*f` formula (using `ctx.noteSample`) for parity first**, then switch to a phase accumulator (enables glide/vibrato; breaks bit-parity — sequence it after the parity milestone). Waveform is the node's config. |
| `ADSRNode` | `ADSRProfile` | stage + elapsed (+ maybe an `ended` flag) | 0-in/1-out (a 0..1 control signal). Advances from `ctx.gate`. A/D/S/R are config from the instrument. |
| `VCA` | `AudioNote`'s `sample*amp*strength` | none | 2-in (audio, gain)/1-out; multiplies, applies `ctx.velocity`. |
| `MixerNode` | `AudioChannel::combineActiveNotes` | none | N-in/1-out sum (for multi-oscillator synthesis). |
| `PanNode` | `AudioChannel::panMonoSample` | none | 1-in/**2-out** (L,R) using `ctx.pan`. |
| `GainNode` | channel/synth `volume` | none | 1-in/1-out scalar multiply. |
| `DelayNode` | (new) | ring buffer (N doubles + index) | First buffer-backed node; validates separable state (per-instance in a voice, shared in post-FX). |
| `FilterNode` (later) | (new) | filter history | HPF/LPF; a "change the sound" node. |

`AudioNote` does **not** become a node — it decomposes into synthesis → `ADSRNode → VCA` inside the
voice graph, and its per-note identity becomes an **instance** (an index into the instrument's pool).

## Open sub-decisions (settle during Phase 2)

- **Instance reclamation** — how the instrument knows a note has finished so it can free the
  instance. Pragmatic first cut: the instrument reclaims when `gate` has been off for the config's
  release duration. Cleaner alternative: the `ADSRNode` writes an `ended` flag into a reserved state
  slot the instrument reads via a known handle. Start with the timer; move to the flag if needed.
- **Pan / mono→stereo placement** — a `PanNode` (1→2) inside the channel graph vs. the channel doing
  the mono→`AudioSample` pan in plain code (matching `panMonoSample`). Lean to plain code first;
  promote to a node when post-FX needs stereo.

## Phases (each compiles, runs, is verifiable)

- **Phase 0 — Core.** ✅ Done (`Signal`/`AudioContext`/`SignalProcessor`/`SignalGraph`/`AudioPipeline`
  + smoke test on `audio-pipeline-bridge`).
- **Phase 1 — DSP nodes + one note.** Implement `OscillatorNode` (parity `t*f`), `ADSRNode`, `VCA`.
  Build a one-oscillator voice graph; render a single instance to a WAV; compare against one
  `AudioNote`.
- **Phase 2 — Instrument + polyphony.** `Instrument` (voice `AudioPipeline` + instance pool + note
  API + `InstrumentConfig`); settle instance reclamation. Prove a chord renders correctly from one
  compiled graph rendered as several instances.
- **Phase 3 — Channel + synth = parity milestone.** `Channel` (instrument + pan/volume → stereo) and
  the synth summing channels, driven by the existing MIDI visitor. Render `moonlight-sonata.json`
  through the pipeline and compare to the procedural oracle. Keep the `t*f` oscillator here.
- **Phase 4 — Change the sound.** Post-FX chain, `DelayNode`, `FilterNode`, phase-accumulator
  oscillator with glide; demonstrate composing nodes to alter playback. Retire the procedural path.
- **Phase 5 — Deferred.** Real-time: `blockSize` > 1, lock-free MIDI event queue, output ring
  buffer; then the pipeline DSL (text → `SignalGraph`).

## Verification

- Build with the documented toolchain (MinGW-w64 GCC + CMake + Ninja):
  `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build`.
- Keep `synthesizeMidiRoll()` (procedural) alongside a new `synthesizeMidiRollViaPipeline()` through
  Phases 1–3. At the parity milestone, render both to WAV and compare — with the `t*f` oscillator and
  identical ADSR/mix maths, aim for sample-exact (or near-exact) output; investigate divergence
  before proceeding. After the Phase-4 phase-accumulator switch, compare audibly / by envelope shape.
- Each phase must run `main.cpp` end-to-end (the graph smoke test + a synthesis pass) and exit 0.
