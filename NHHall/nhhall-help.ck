// NHHall is an open source algorithmic reverb unit, developed by Nathan Ho in
// 2018 for SuperCollider, ChucK, and Auraglyph. Features:
//
// - Allpass loop topology with delay line modulaton for a lush 90's IDM sound
// - True stereo signal path with controllable spread
// - Infinite hold support
// - Respectable CPU use
// - Sample-rate independence
// - Permissive MIT license
//
// The project repo may be found at https://github.com/snappizz/nh-ugens

// Options:
// rt60: (float), default 1::second
// stereo: (float) [0.0 - 1.0], default 0.5
// lowFreq: (float), default 200
// lowRatio: (float), default 0.5
// hiFreq: (float), default 4000
// hiRatio: (float), default 0.5
// earlyDiffusion: (float) [0.0 - 1.0], default 0.5
// lateDiffusion: (float) [0.0 - 1.0], default 0.5
// modRate: (float) [0.0 - 1.0], default 0.2
// modDepth: (float) [0.0 - 1.0], default 0.3

adc => NHHall verb => dac;

30.0 => verb.rt60;

while (true) {
    second => now;
}
