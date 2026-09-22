//-----------------------------------------------------------------------------
// Test MIDI Playback with Time-based Position Markers
// Maps different MIDI notes to different slice positions in SovereignBreak.wav
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/SovereignBreak.wav") == 0) {
    <<< "FAIL: could not load SovereignBreak.wav" >>>;
    me.exit();
}

<<< "=== MIDI Slice Playback Test ===" >>>;
<<< "File duration:", sampler.endMarker(), "seconds" >>>;
<<< "" >>>;

// Get file duration
sampler.endMarker() => float duration;

// Define slice positions in seconds
[0.0, 0.371, 0.558, 0.935, 1.495, 1.873, 2.436, duration] @=> float slicePositions[];

// Map to MIDI notes C3-C4 (48-60)
48 => int baseNote;

// Note tracking is OFF by default, so all slices play at original pitch
// (Each MIDI note only affects start position, not pitch)

// Set up note positions in seconds
for (0 => int i; i < slicePositions.size(); i++) {
    if (i < slicePositions.size() - 1) {
        // Set start marker to this position, end marker to next position
        slicePositions[i] => sampler.startMarker;
        slicePositions[i + 1] => sampler.endMarker;

        // Map note to start at this slice position (in seconds)
        sampler.notePos(baseNote + i, slicePositions[i]);

        <<< "MIDI Note", baseNote + i, "-> slice", i,
            "(", slicePositions[i], "-", slicePositions[i + 1], "seconds )" >>>;
    }
}

// Reset markers to full range for playback
0.0 => sampler.startMarker;
duration => sampler.endMarker;

<<< "" >>>;
<<< "Playing each slice sequentially..." >>>;
<<< "" >>>;

// Play each slice
for (0 => int i; i < slicePositions.size() - 1; i++) {
    // Set the playback range for this slice
    slicePositions[i] => sampler.startMarker;
    slicePositions[i + 1] => sampler.endMarker;

    <<< "Playing slice", i, "(note", baseNote + i, ")" >>>;
    sampler.noteOn(baseNote + i, 127);
    500::ms => now;
    sampler.noteOff();
    200::ms => now;
}

1::second => now;

<<< "" >>>;
<<< "Now playing a pattern using the slices..." >>>;
<<< "" >>>;

// Reset to full range
0.0 => sampler.startMarker;
duration => sampler.endMarker;

// Play a rhythmic pattern using different slices
[0, 2, 0, 4, 1, 3, 1, 5, 0, 2, 0, 6, 1, 3, 1, 4] @=> int pattern[];

for (0 => int i; i < pattern.size(); i++) {
    pattern[i] => int sliceIdx;

    if (sliceIdx < slicePositions.size() - 1) {
        // Set the slice range
        slicePositions[sliceIdx] => sampler.startMarker;
        slicePositions[sliceIdx + 1] => sampler.endMarker;

        sampler.noteOn(baseNote + sliceIdx, 100 + (i % 3) * 9);  // Vary velocity
        120::ms => now;
        sampler.noteOff();
        30::ms => now;
    }
}

1::second => now;

<<< "" >>>;
<<< "Testing with pitch variations..." >>>;
<<< "" >>>;

// Play slices with different pitches
for (0 => int i; i < 7; i++) {
    if (i < slicePositions.size() - 1) {
        slicePositions[i] => sampler.startMarker;
        slicePositions[i + 1] => sampler.endMarker;

        (i - 3) * 2.0 => sampler.transpose;  // Pitch variation

        sampler.noteOn(baseNote + i, 120);
        300::ms => now;
        sampler.noteOff();
        100::ms => now;
    }
}

0.0 => sampler.transpose;  // Reset pitch

1::second => now;

<<< "" >>>;
<<< "MIDI slice test complete!" >>>;
