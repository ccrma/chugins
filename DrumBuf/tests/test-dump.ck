//-----------------------------------------------------------------------------
// Test dump() Method
// Verifies that .read() is silent and .dump() prints file info
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

<<< "=== Testing dump() Method ===\n" >>>;

<<< "Test 1: dump() before loading (should print 'No file loaded')" >>>;
sampler.dump();

<<< "\nTest 2: .read() should be silent (no automatic output)" >>>;
<<< "Loading file..." >>>;
if (sampler.read(me.dir() + "/snare.wav") == 0) {
    <<< "FAIL: could not load snare.wav" >>>;
    me.exit();
}
<<< "File loaded (notice: no automatic output)" >>>;

<<< "\nTest 3: Explicitly call .dump() to see file info" >>>;
sampler.dump();

<<< "\nTest 4: Query methods return same info" >>>;
<<< "  Channels:", sampler.channels() >>>;
<<< "  Sample rate:", sampler.samplerate() >>>;
<<< "  Samples:", sampler.samples() >>>;
<<< "  Length:", sampler.length(), "seconds" >>>;
<<< "  Filename:", sampler.filename() >>>;

<<< "\nTest 5: Load another file and dump()" >>>;
sampler.read(me.dir() + "/SovereignBreak.wav");
<<< "(Still silent - no automatic output)" >>>;
<<< "\nCall dump() to see new file info:" >>>;
sampler.dump();

<<< "\n=== dump() Test Complete ===" >>>;
<<< ".read() is now silent by default." >>>;
<<< "Call .dump() when you want to see file information." >>>;
