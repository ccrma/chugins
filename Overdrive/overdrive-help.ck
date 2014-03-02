///////////////////////////////////////////////////
// Overdrive                                     //
//                                               //
// Simple overdrive distortion created by        //
// applying a non-linear transfer function to    //
// the input signal.                             //
// Adapted from cyclone/overdrive~ from Pd       //
///////////////////////////////////////////////////
//
// Settings:
//
// drive (float): set overdrive amount.
//       1 is no distortion
//       >1 simulates overdrive
//       0-1 simulates... underdrive? I guess?
//

Sitar pluck => Overdrive od => dac;
[36, 39, 40, 43, 44, 47, 48] @=> int notes[];
1 => float driveVal;
repeat (24)
{
	notes[Math.random2(0,notes.size()-1)] => Std.mtof => pluck.freq;
	1 => pluck.noteOn;
	second => now;
	1 +=> driveVal;
	<<< "drive:", driveVal => od.drive >>>;
}
second => now;