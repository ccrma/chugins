// Create a wav file to examine each interpolation algorithm.

Wavetable w => dac => WvOut rec => blackhole;
"test" => rec.wavFilename;

1000 => w.freq;
[-1.0,1] @=> float myTable[];
w.setTable(myTable);

for (int i; i<5; i++)
{
  i => w.interpolate;
  2::second => now;
  w =< dac;
  second => now;
  w => dac;
}

