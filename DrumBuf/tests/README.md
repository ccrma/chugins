# DrumBuf Tests

This directory contains test scripts and audio files for the DrumBuf chugin.

## Test Files

Run everything headless with `tests/run-tests.sh` (see below). `test-semantics.ck` is the
only file with machine-checked assertions; the rest are listening demos.

- **DrumBuf-debug.ck** - Extended listening suite with all modes
- **DrumBuf-test.ck** - Full test suite covering all features (has PASS/FAIL prints)
- **jungle-snare-rolls.ck** - Musical demo: snare rolls, pitch ramps
- **test-constructor.ck** - Tests both old and new constructor syntax
- **test-dump.ck** - Test dump() Method
- **test-envelope.ck** - Test AR Envelope
- **test-exponential-envelope.ck** - Test Exponential Envelope
- **test-gain.ck** - Test Gain Control
- **test-hold-envelope.ck** - Test Hold Envelope
- **test-instant-envelope.ck** - Test Instant Attack/Release (0.0 seconds)
- **test-interpolation.ck** - DrumBuf Interpolation Test Script
- **test-markers.ck** - Test Start/End Markers
- **test-midi-slices.ck** - Test MIDI Playback with Time-based Position Markers
- **test-notetracking.ck** - DrumBuf Note Tracking Test
- **test-patch-crash.ck** - Patch chugin driving `transpose`, then `disconnect()` (needs Patch.chug; crashed with old Patch, see KNOWN-ISSUES)
- **test-patch-no-env.ck** - Patch modulation without envelope (needs Patch.chug)
- **test-pingpong.ck** - Test Ping-Pong Looping Mode
- **test-playback-modes.ck** - Exercise all four playbackMode values (ONESHOT, LOOP, PINGPONG, THRU)
- **test-polyphony-basic.ck** - Test Basic Polyphony
- **test-polyphony-chords.ck** - Test Polyphony with Chords
- **test-polyphony-verify.ck** - Test Polyphony Verification
- **test-query.ck** - Test Query Methods
- **test-reverse.ck** - Test Reverse Playback
- **test-samplerate.ck** - Test query methods (matching SndBuf API)
- **test-semantics.ck** - **Automated** structural checks (activeVoices + timing): slice auto-stop, reverse plays its own slice, rate, noteOff in LOOP, envelope release, polyphony shrink, THRU, decay/fadeOut, degenerate loop points, positions past EOF. Prints PASS/FAIL.
- **test-stereo.ck** - This test loads a stereo file and plays it to verify stereo functionality
- **test-thru-monophonic.ck** - Test THRU Mode Monophonic Behavior
- **test-thru.ck** - Test Thru Mode
- **test-timestretch.ck** - Test Time Stretching (should NOT change pitch)
- **test-velocity.ck** - Test Velocity Sensitivity

## Audio Files
Every test loads one of these two files from the tests directory (`me.dir()`);
a test that cannot load its file prints a `FAIL:` line and exits.
- **SovereignBreak.wav** - Sample drum loop for testing
- **snare.wav** - Single snare hit (jungle-snare-rolls, polyphony, envelope and velocity tests)

## Running Tests

Headless, all at once (needs a built `../DrumBuf.chug`; runs faster than real time):

```bash
tests/run-tests.sh
```


From the DrumBuf directory:

```bash
# Run individual tests
chuck --chugin:DrumBuf.chug tests/test-constructor.ck
chuck --chugin:DrumBuf.chug tests/test-query.ck
chuck --chugin:DrumBuf.chug tests/test-playback-modes.ck
chuck --chugin:DrumBuf.chug tests/test-reverse.ck
chuck --chugin:DrumBuf.chug tests/test-pingpong.ck
chuck --chugin:DrumBuf.chug tests/test-timestretch.ck
chuck --chugin:DrumBuf.chug tests/test-gain.ck
chuck --chugin:DrumBuf.chug tests/test-envelope.ck
chuck --chugin:DrumBuf.chug tests/test-velocity.ck
chuck --chugin:DrumBuf.chug tests/test-markers.ck
chuck --chugin:DrumBuf.chug tests/test-thru.ck

# Run comprehensive test suites
chuck --chugin:DrumBuf.chug tests/DrumBuf-test.ck
chuck --chugin:DrumBuf.chug tests/DrumBuf-debug.ck
```

Or from the tests directory:

```bash
cd tests
chuck --chugin:../DrumBuf.chug test-reverse.ck
chuck --chugin:../DrumBuf.chug test-pingpong.ck
chuck --chugin:../DrumBuf.chug test-timestretch.ck
```

## What to Listen For

### test-constructor.ck
- Tests three ways to create a DrumBuf instance:
  - Test 1: `DrumBuf sampler("file.wav")` - Load file directly in constructor (new syntax)
  - Test 2: `DrumBuf sampler; sampler.read("file.wav")` - Load file after construction (old syntax)
  - Test 3: `DrumBuf sampler("")` - Empty string in constructor (should not crash)
- All three methods should work and play the same snare sound
- Validates backward compatibility while adding convenience syntax

### test-query.ck
- Tests all query methods for introspection
  - Test 1: Query before loading (should return safe defaults)
  - Test 2: Query after loading (should return file metadata)
  - Test 3: Verify duration accuracy
  - Test 4: Query with constructor-loaded file
- Tests `length()`, `channels()`, `isLoaded()`, and `filename()` methods
- Validates file metadata is correctly stored and accessible

### test-playback-modes.ck
- Comprehensive test of the new unified playbackMode API
  - Test 1: ONESHOT (0) - plays once, stops at end marker
  - Test 2: LOOP (1) - loops between loop start/end points
  - Test 3: PINGPONG (2) - bounces between loop points
  - Test 4: THRU (3) - plays through end marker to EOF
  - Test 5: Reverse + ONESHOT - plays backward
  - Test 6: Reverse + THRU - plays backward from EOF to 0
- Validates all four playback modes and reverse combinations
- Quick verification that the refactored API works correctly

### test-reverse.ck
- Tests the reverse playback flag for both one-shot and looped playback
  - Test 1: Forward one-shot (reference)
  - Test 2: Reverse one-shot - plays backward from end to start
  - Test 3: Forward loop (reference)
  - Test 4: Reverse loop - loops backward from end to start
  - Test 5: Reverse loop at half speed
- Validates that reverse flag works independently of loop mode

### test-pingpong.ck
- Test 1: Forward loop (reference)
- Test 2: Ping-pong - should bounce back and forth smoothly
- Test 3: Ping-pong faster - quicker bounces
- Test 4: Ping-pong with pitch - bounces with higher pitch

### test-timestretch.ck
- All tests should maintain the SAME pitch
- Only the duration should change
- 0.5x = twice as long, 2.0x = half as long

### test-gain.ck
- Tests output gain control in decibels
- 0 dB = unity gain (no change)
- -6 dB ≈ half volume, +6 dB ≈ double volume
- Demonstrates volume control independent of velocity

### test-envelope.ck
- Tests AR (attack-release) envelope
- Envelope defaults to OFF
- When enabled, noteOn() starts attack phase, noteOff() starts release
- Tests various attack/release times
- Shows envelope with looping and percussive patterns

### test-velocity.ck
- Tests velocity sensitivity control
- Compares different sensitivity curves: 0 (none), 0.5 (compressed), 1.0 (linear), 2.0 (expanded), 4.0 (extreme)
- Demonstrates how velocity curve affects dynamic range
- Formula: `output = pow(velocity/127, sensitivity)`

### test-markers.ck
- Tests start/end marker functionality (sample-rate independent)
- Test 1: Full file playback with default markers
- Test 2: First half only (end marker = 0.5 seconds)
- Test 3: Second half only (start marker = 0.5 seconds)
- Test 4: Middle section (0.3 - 0.7 seconds)
- Test 5: Short looped segment (0.1 - 0.3 seconds with looping)
- Test 6: Note positions mapped within marker range
- Test 7: Tiny segment with time stretch for granular effect
- Demonstrates how markers define the playable region and how note positions work within that range

### test-thru.ck
- Tests thru mode functionality
- Test 1: Without thru mode (stops at end marker = 0.5 seconds)
- Test 2: With thru mode enabled (plays from start to end of file, ignoring end marker)
- Test 3: MIDI slicing with thru mode (start at different positions, play through to end)
- Test 4: Practical drum slicing example (trigger at specific hit, let it ring out)
- Demonstrates how thru mode allows natural decay when slicing drum breaks

## Adding Your Own Samples

Replace `SovereignBreak.wav` with any audio file, or modify the test scripts to load different files.

Supported formats: WAV, FLAC, OGG, MP3, and more (via libsndfile).
