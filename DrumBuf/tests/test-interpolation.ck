//-----------------------------------------------------------------------------
// DrumBuf Interpolation Test Script
// Tests drop/linear/Lagrange interpolation quality
//-----------------------------------------------------------------------------

// Create a DrumBuf instance
DrumBuf sampler => dac;

// Load a test audio file
if (sampler.read(me.dir() + "/snare.wav") == 0) {
    <<< "FAIL: could not load snare.wav" >>>;
    me.exit();
}

<<< "=== DrumBuf Interpolation Mode Test ===" >>>;
<<< "" >>>;
<<< "This test demonstrates the difference between DROP, LINEAR, and LAGRANGE interpolation." >>>;
<<< "Higher modes provide better quality, especially noticeable with pitch-shifting." >>>;
<<< "" >>>;

//-----------------------------------------------------------------------------
// Test 1: Drop interpolation (no interpolation) with pitch shift
//-----------------------------------------------------------------------------
<<< "Test 1: DROP interpolation (none) with +7 semitones pitch shift" >>>;
0 => sampler.interp;  // Set to DROP mode (most aliasing)
7.0 => sampler.transpose;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

//-----------------------------------------------------------------------------
// Test 2: Linear interpolation (default) with pitch shift
//-----------------------------------------------------------------------------
<<< "Test 2: LINEAR interpolation (default) with +7 semitones pitch shift" >>>;
1 => sampler.interp;  // Set to LINEAR mode
7.0 => sampler.transpose;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

//-----------------------------------------------------------------------------
// Test 3: Lagrange interpolation with pitch shift
//-----------------------------------------------------------------------------
<<< "Test 3: LAGRANGE interpolation with +7 semitones pitch shift" >>>;
2 => sampler.interp;  // Set to LAGRANGE mode
7.0 => sampler.transpose;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

//-----------------------------------------------------------------------------
// Test 4: Drop interpolation with extreme pitch shift
//-----------------------------------------------------------------------------
<<< "Test 4: DROP interpolation with +12 semitones (1 octave up)" >>>;
0 => sampler.interp;  // DROP
12.0 => sampler.transpose;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

//-----------------------------------------------------------------------------
// Test 5: Linear interpolation with extreme pitch shift
//-----------------------------------------------------------------------------
<<< "Test 5: LINEAR interpolation with +12 semitones (1 octave up)" >>>;
1 => sampler.interp;  // LINEAR
12.0 => sampler.transpose;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

//-----------------------------------------------------------------------------
// Test 6: Lagrange interpolation with extreme pitch shift
//-----------------------------------------------------------------------------
<<< "Test 6: LAGRANGE interpolation with +12 semitones (1 octave up)" >>>;
2 => sampler.interp;  // LAGRANGE
12.0 => sampler.transpose;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

//-----------------------------------------------------------------------------
// Test 7: Verify getter works
//-----------------------------------------------------------------------------
<<< "" >>>;
<<< "Test 7: Verify getter" >>>;
0 => sampler.interp;
<<< "  Set to DROP (0), getter returns:", sampler.interp() >>>;
1 => sampler.interp;
<<< "  Set to LINEAR (1), getter returns:", sampler.interp() >>>;
2 => sampler.interp;
<<< "  Set to LAGRANGE (2), getter returns:", sampler.interp() >>>;

<<< "" >>>;
<<< "=== Interpolation test completed! ===" >>>;
<<< "" >>>;
<<< "Summary:" >>>;
<<< "  0 = DROP (none, fastest, most aliasing)" >>>;
<<< "  1 = LINEAR (default, good balance)" >>>;
<<< "  2 = LAGRANGE (4-point cubic, highest quality)" >>>;
<<< "" >>>;
<<< "Lagrange interpolation should sound clearer and less aliased," >>>;
<<< "especially at higher pitch shifts." >>>;
