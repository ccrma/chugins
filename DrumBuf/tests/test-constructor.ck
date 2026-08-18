// test-constructor.ck - Test constructor with filename parameter
// Tests both old and new constructor syntax

<<< "=== Testing DrumBuf Constructor ===" >>>;
<<< "" >>>;

// Test 1: Constructor with filename (new syntax)
<<< "Test 1: Constructor with filename - DrumBuf sampler1(\"snare.wav\") => dac;" >>>;
DrumBuf sampler1(me.dir() + "/snare.wav") => dac;
sampler1.noteOn(60, 100);
1::second => now;
<<< "SUCCESS: Constructor with filename works!" >>>;
<<< "" >>>;

// Test 2: Constructor without filename (old syntax)
<<< "Test 2: Constructor without filename - DrumBuf sampler2 => dac;" >>>;
DrumBuf sampler2 => dac;
sampler2.read(me.dir() + "/snare.wav");
sampler2.noteOn(60, 100);
1::second => now;
<<< "SUCCESS: Constructor without filename works!" >>>;
<<< "" >>>;

// Test 3: Constructor with empty string (should not crash)
<<< "Test 3: Constructor with empty string - DrumBuf sampler3(\"\") => dac;" >>>;
DrumBuf sampler3("") => dac;
sampler3.read(me.dir() + "/snare.wav");
sampler3.noteOn(60, 100);
1::second => now;
<<< "SUCCESS: Constructor with empty string works!" >>>;
<<< "" >>>;

<<< "=== All constructor tests passed! ===" >>>;
