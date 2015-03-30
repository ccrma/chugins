// PowerADSR is a power based ADSR envelope that 
// allows separate power curves for each envelope phase 

// ~options
//
// set (duration, duration, float, duration), default (0.0, 0.0, 1.0, 0.0)
//   sets duration for attack, decay, and release phase; sets sustain level
//
// attack (duration), default 0.0
//   sets length for the attack phase
//
// decay (duration), default 0.0
//   sets length for the decay phase
//
// sustain (float), default 1.0
//   sets level for the sustain phase
//
// setCurves (float, float, float), default (1.0, 1.0, 1.0) (all linear)
//   sets power curve for attack, decay, and release phase
//
// release (duration), default 0.0
//   sets length for the release phase
//
// attackCurve (float), default 1.0 (linear)
//   sets power curve for the attack phase
//
// decayCurve (float), default 1.0 (linear)
//   sets power curve for the decay phase
//
// releaseCurve(float), default 1.0 (linear)
//   sets power curve for the release phase

// set number of sines and power envs
5 => int num;
SinOsc sin[num];
PowerADSR env[num];

// sound chain
for (int i; i < num; i++) {
    sin[i] => env[i] => dac;
    sin[i].gain(1.0/num);
}

fun void loopSin(int idx) {
    dur env_time;
    float curve;
    while (true) {
        Math.random2(5, 30)::second => env_time;

        // curves below 1.0 will be "hill" shaped, 
        // curves above 1.0 will resemble an exponential curve
        Math.random2f(0.0, 4.0) => curve;
        sin[idx].freq(Math.random2f(400, 440));

        // set all envelope durations
        env[idx].set(env_time, 1::second, 1.0, env_time);
        // set all envelope power curves 
        env[idx].setCurves(curve, 1.0, 1.0/curve);

        // begins attack phase 
        env[idx].keyOn(); 
        env_time => now;

        // begins decay phase
        1::second => now;

        // begins release phase
        env[idx].keyOff(); 
        env_time => now;
    }
}

for (int i; i < num; i++) {
    spork ~ loopSin(i);
}

while (true) {
    1::second => now;
}
