//-----------------------------------------------------------------------------
// Test Velocity Sensitivity
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/SovereignBreak.wav") == 0) {
    <<< "FAIL: could not load SovereignBreak.wav" >>>;
    me.exit();
}

<<< "=== Testing Velocity Sensitivity ===" >>>;
<<< "" >>>;

// Test with different velocity values
int velocities[5];
[40, 60, 80, 100, 127] @=> velocities;

// Test 1: Linear sensitivity (default)
<<< "Test 1: Linear velocity response (sensitivity = 1.0)" >>>;
1.0 => sampler.velocitySens;
<<< "Current sensitivity:", sampler.velocitySens() >>>;

for (0 => int i; i < velocities.cap(); i++) {
    <<< "  Velocity", velocities[i] >>>;
    sampler.noteOn(60, velocities[i]);
    500::ms => now;
    sampler.noteOff();
    300::ms => now;
}

1::second => now;

// Test 2: No sensitivity (all velocities sound the same)
<<< "Test 2: No sensitivity (all velocities should sound the same)" >>>;
0.0 => sampler.velocitySens;
<<< "Current sensitivity:", sampler.velocitySens() >>>;

for (0 => int i; i < velocities.cap(); i++) {
    <<< "  Velocity", velocities[i], "(should be same volume)" >>>;
    sampler.noteOn(60, velocities[i]);
    500::ms => now;
    sampler.noteOff();
    300::ms => now;
}

1::second => now;

// Test 3: Exponential sensitivity (more dynamic range)
<<< "Test 3: Exponential sensitivity = 2.0 (exaggerated dynamics)" >>>;
2.0 => sampler.velocitySens;
<<< "Current sensitivity:", sampler.velocitySens() >>>;

for (0 => int i; i < velocities.cap(); i++) {
    <<< "  Velocity", velocities[i] >>>;
    sampler.noteOn(60, velocities[i]);
    500::ms => now;
    sampler.noteOff();
    300::ms => now;
}

1::second => now;

// Test 4: Very high sensitivity (extreme dynamics)
<<< "Test 4: Very high sensitivity = 4.0 (extreme dynamics)" >>>;
4.0 => sampler.velocitySens;
<<< "Current sensitivity:", sampler.velocitySens() >>>;

for (0 => int i; i < velocities.cap(); i++) {
    <<< "  Velocity", velocities[i] >>>;
    sampler.noteOn(60, velocities[i]);
    500::ms => now;
    sampler.noteOff();
    300::ms => now;
}

1::second => now;

// Test 5: Compressed dynamics (less sensitive)
<<< "Test 5: Compressed dynamics = 0.5 (less sensitive)" >>>;
0.5 => sampler.velocitySens;
<<< "Current sensitivity:", sampler.velocitySens() >>>;

for (0 => int i; i < velocities.cap(); i++) {
    <<< "  Velocity", velocities[i] >>>;
    sampler.noteOn(60, velocities[i]);
    500::ms => now;
    sampler.noteOff();
    300::ms => now;
}

<<< "" >>>;
<<< "Velocity sensitivity test complete" >>>;
<<< "" >>>;
<<< "Velocity curve formula: output = pow(velocity/127, sensitivity)" >>>;
<<< "  sensitivity = 0.0: No dynamics (all vel = 1.0)" >>>;
<<< "  sensitivity = 0.5: Compressed (quieter notes louder)" >>>;
<<< "  sensitivity = 1.0: Linear (default)" >>>;
<<< "  sensitivity = 2.0: Expanded (more dynamic range)" >>>;
<<< "  sensitivity > 2.0: Extreme dynamics" >>>;
