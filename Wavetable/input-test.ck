Step s => Envelope e => Wavetable w => dac;
1 => s.next;
440 => e.value;
4400 => e.target;
20::second => e.duration;
0.75 => w.gain;
20::second => now;
220 => e.target;
20::second => now;
