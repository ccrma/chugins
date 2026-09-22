# DrumBuf

A drum sampler chugin — a companion to SndBuf/WarpBuf in the spirit of Ableton
Live 12's Drum Sampler and Amiga-era samplers. Two overlapping Hann-windowed
grains give **independent pitch-shifting and time-stretching** (the classic
90s jungle technique), and a per-note **slice map** turns any break into a
playable kit.

`Sampler.h`, `Sampler.cpp` and `PolyVoiceManager.h` are a host-agnostic C++
core (no ChucK types); `DrumBuf-chuck.cpp` is the ChucK glue. Filters,
saturation and effects are deliberately *not* inside: patch DrumBuf into
`LPF`, the Faust chugin, etc.

## Features

- Stereo output (mono files are duplicated); files at any sample rate keep
  their pitch and duration
- `transpose` (semitones) and `rate` (time-stretch) are independent;
  `grainSize` 1–1000 ms; drop / linear / Lagrange interpolation
- Playback modes ONESHOT / LOOP / PINGPONG / THRU, each with `reverse`
- Slice map: `notePos(note, seconds)` — in ONESHOT a voice plays its slice
  `[pos, next higher note position)` and auto-stops; reversed, it plays the
  same slice backwards
- Start/end markers and loop points; optional exponential AHR envelope;
  per-hit `decay` (exponential fade) and `fadeOut` (linear ramp before the
  slice end, so cuts don't click)
- Polyphony up to 64 voices, four stealing policies, `activeVoices()`;
  velocity curve; note tracking (off / traditional / granular) with `rootNote`
- Loads anything libsndfile reads (WAV, FLAC, OGG, MP3, …)

## Building

Needs libsndfile via `pkg-config` (`brew install libsndfile` /
`apt install libsndfile1-dev`). On Windows, build `DrumBuf.vcxproj` with
libsndfile installed under `C:\Program Files\libsndfile` (or pass
`/p:LibSndFileDir=...`); the resulting `DrumBuf.chug` needs `sndfile.dll` on
the PATH.

```bash
cd DrumBuf
make mac        # or: make linux
make install    # copies DrumBuf.chug to /usr/local/lib/chuck (override CHUGIN_PATH)
```

## Usage

```chuck
DrumBuf s("break.wav") => dac;   // or: DrumBuf s => dac; s.read("break.wav");

s.noteOn(60, 127);               // MIDI note, velocity
2::second => now;
s.noteOff();
```

Pitch and time, separately:

```chuck
5.0 => s.transpose;   // up a fourth
0.5 => s.rate;        // half speed, same pitch
0.03 => s.grainSize;  // 30 ms grains: more texture; 100 ms: smoother
```

A slice map — one `notePos` per slice, then play the slices like drum pads
(the auto-stop at the next position keeps them from bleeding into each other):

```chuck
[0.0, 0.371, 0.558, 0.935, 1.495] @=> float starts[];
for (0 => int i; i < starts.size(); i++) s.notePos(48 + i, starts[i]);
0.01 => s.fadeOut;                 // 10 ms ramp at every slice end

s.noteOn(50, 127); 250::ms => now; // slice 2
1 => s.reverse;
s.noteOn(50, 127); 250::ms => now; // slice 2, backwards
```

Polyphony, envelope, note tracking:

```chuck
16 => s.polyphony;         // up to 64
2 => s.noteTracking;       // GRANULAR: pitch follows the note, duration doesn't
60 => s.rootNote;
1 => s.envelope; 0.005 => s.attack; 0.2 => s.release;
s.noteOn(60, 127); s.noteOn(64, 120); s.noteOn(67, 115);
2::second => now;
s.noteOff(64);             // release one; s.noteOff() releases all
1::second => now;
s.allNotesOff();           // hard stop
```

Automation: every parameter is a plain setter, so drive it from a shred
(`lfo.last() => s.transpose; 1::samp => now;`), from `Line`, or from `Patch`
(see KNOWN-ISSUES.md for the Patch `disconnect()` caveat).

## API

Every setter returns the value it set; the same name with no argument is the
getter. Times are seconds, positions in `notePos`/markers are seconds into the
file, `loopStart`/`loopEnd` are normalized 0–1.

| Group | Methods |
|---|---|
| File | `read(path)`, `isLoaded()`, `filename()`, `channels()`, `samplerate()`, `samples()`, `length()`, `dump()` |
| Notes | `noteOn(note, vel)`, `noteOff()`, `noteOff(note)`, `allNotesOff()`, `notePos(note, sec)`, `activeVoices()` |
| Pitch / time | `transpose` (st), `rate`, `grainSize` (s), `interp` (0 drop, 1 linear, 2 Lagrange) |
| Modes | `playbackMode` (0 ONESHOT, 1 LOOP, 2 PINGPONG, 3 THRU), `reverse`, `loopStart`, `loopEnd`, `startMarker`, `endMarker` |
| Envelope / shaping | `envelope`, `attack`, `hold`, `release`, `decay`, `fadeOut`, `velocitySens`, `gain` (dB) |
| Voices | `polyphony` (1–64), `stealingPolicy` (0 oldest, 1 oldest-releasing, 2 quietest, 3 round-robin) |
| Tracking | `noteTracking` (0 off, 1 traditional, 2 granular), `rootNote` |

`noteOff` runs the envelope's release when `envelope` is on (even for a voice
started while it was off) and stops the voice immediately otherwise, in every
mode. `reverse` and `velocitySens` are latched by each voice at `noteOn`;
everything else takes effect on the next sample.

## Testing

```bash
tests/run-tests.sh          # every test headless (chuck --silent), fails on crash or FAIL:
```

`tests/test-semantics.ck` holds machine-checked assertions; the rest are
listening demos — see `tests/README.md`.

## Design notes

Two grains half a grain apart read from the playhead; their Hann windows sum
to exactly 1, so unity gain reproduces the source level and a rate-1 render is
bit-exact. Grain read speed carries the pitch (`transpose`), the playhead hop
carries time (`rate`); the file/system rate ratio scales both. Setters ignore
non-finite values, loop points are used as an ordered pair with a minimum
length, and reads outside the buffer are silence, so a modulator can't drive
the engine into NaN. See KNOWN-ISSUES.md.
