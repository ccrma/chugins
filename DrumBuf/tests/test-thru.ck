//-----------------------------------------------------------------------------
// Test Thru Mode
// Demonstrates how THRU playback mode plays through end marker to EOF
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/SovereignBreak.wav") == 0) {
    <<< "FAIL: could not load SovereignBreak.wav" >>>;
    me.exit();
}

<<< "=== Testing THRU Playback Mode ===" >>>;
<<< "File duration:", sampler.length(), "seconds" >>>;
<<< "" >>>;

// Test 1: ONESHOT mode (default) - stops at end marker
<<< "Test 1: ONESHOT mode (stops at end marker)" >>>;
<<< "Setting end marker to 0.5 seconds" >>>;
0.0 => sampler.startMarker;
0.5 => sampler.endMarker;
0 => sampler.playbackMode;  // ONESHOT
<<< "Playback mode:", sampler.playbackMode() >>>;

sampler.noteOn(60, 127);
1::second => now;  // Waits 1 second but playback stops at 0.5s
sampler.noteOff();
1::second => now;

// Test 2: THRU mode - plays to end of file
<<< "Test 2: THRU mode (continues to end of file)" >>>;
<<< "End marker still at 0.5 seconds, but THRU mode ignores it" >>>;
0.0 => sampler.startMarker;
0.5 => sampler.endMarker;
3 => sampler.playbackMode;  // THRU
<<< "Playback mode:", sampler.playbackMode() >>>;

sampler.noteOn(60, 127);
3.5::second => now;  // Plays entire file despite end marker at 0.5s
sampler.noteOff();
1::second => now;

// Test 3: THRU mode with MIDI slicing - start at slice position, play thru to end
<<< "Test 3: MIDI slicing with THRU mode" >>>;
<<< "Start markers at different positions, play through to end" >>>;

// Define slice positions
[0.0, 0.371, 0.558, 0.935, 1.495] @=> float slicePositions[];

3 => sampler.playbackMode;  // THRU mode

// Play from each slice position to the end of file
for (0 => int i; i < slicePositions.size(); i++) {
    <<< "Playing from", slicePositions[i], "seconds to end of file" >>>;

    slicePositions[i] => sampler.startMarker;
    sampler.length() => float length;  // Get file length
    length => sampler.endMarker;  // Reset end marker to full length

    sampler.noteOn(60, 120);
    1.5::second => now;
    sampler.noteOff();
    300::ms => now;
}

1::second => now;

// Test 4: Practical use case - trigger drum hits that ring out
<<< "Test 4: Drum hits with THRU mode (realistic playback)" >>>;
<<< "Slicing a break into hits, each plays through to end" >>>;

// Set up slice positions for individual drum hits
0.371 => sampler.startMarker;  // Start at snare hit
0.558 => sampler.endMarker;    // End marker (ignored with THRU)
3 => sampler.playbackMode;  // THRU

<<< "Playing snare hit, but it rings out through the rest of the sample" >>>;
sampler.noteOn(60, 127);
3::second => now;
sampler.noteOff();

<<< "" >>>;
<<< "THRU mode test complete!" >>>;
<<< "" >>>;
<<< "Summary:" >>>;
<<< "- ONESHOT: Playback stops at end marker" >>>;
<<< "- THRU: Playback continues to end of file, ignoring end marker" >>>;
<<< "- Useful for drum slicing where you want hits to ring out naturally" >>>;
