////////////////////////////////////////////////////
// Wavetable - User-definable Wavetable with      //
//            various interpolation algorithms    //
//                                                //
// by Joel Matthys                                //
// (c) 2016, GPL 2.0                              //
//                                                //
////////////////////////////////////////////////////
//
// Settings:
// freq (float): set frequency
// interpolate (int): 0 = no interpolation
//                    1 = linear interpolation
//                    2 = cubic interpolation
//                    3 = Hermite interpolation
// setTable (float[]): associate a ChucK array with the Wavetable instrument
// sync (int): 0 = connected input UGen controls frequency
//             1 = connected input UGen (usually Phasor) controls phase
//             * see input-test.ck for examples
//
// By default the Wavetable instrument uses a 2048-point non-interpolating sine table.

Wavetable w => dac => WvOut rec => blackhole;
"test" => rec.wavFilename;
0.8 => w.gain;
440 => w.freq;

<<< "built-in sine table, non-interpolating", "" >>>;
2::second => now;

w =< dac;
ms => now;
w => dac;

// Using a ChucK array as a wavetable
[-1.0, -0.5, 0, 0.5, 1, 0.5, 0, -0.5] @=> float myTable[];

w.setTable (myTable);
// ChucK array "myTable" is now associated with the w Wavetable instrument.
<<< "Using array myTable as the wavetable", "">>>;
<<< "Non-interpolating", "">>>;
2::second => now;

w =< dac;
ms => now;
w => dac;

1 => w.interpolate; // Linear Interpolation
<<< "Linear interpolation", "" >>>;
2::second => now;

w =< dac;
ms => now;
w => dac;

2 => w.interpolate; // Lagrange interpolation
<<< "Lagrange interpolation", "" >>>;
2::second => now;

w =< dac;
ms => now;
w => dac;

3 => w.interpolate; // Cubic interpolation
<<< "Cubic interpolation", "" >>>;
2::second => now;

w =< dac;
ms => now;
w => dac;

4 => w.interpolate; // Hermite interpolation
<<< "Hermite interpolation", "" >>>;
2::second => now;

w =< dac;
ms => now;
w => dac;

<<< "Modifying the wavetable on the fly!", "">>>;
// Any changes to myTable will change the waveform!
myTable.size() => int tabsize;
repeat (15)
{
	Math.random2f(-1,1) => myTable[Math.random2(0,tabsize-1)]; // randomly change one of the values in the table
	1::second => now;
}
