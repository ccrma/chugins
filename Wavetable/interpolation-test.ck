Wavetable w => dac => WvOut rec => blackhole;
"test" => rec.wavFilename;

2 => w.freq;
[-1.0,0,1,0] @=> float myTable[];
w.setTable(myTable);

for (int i; i<4; i++)
{
  i => w.interpolate;
  second => now;
  w =< dac;
  50::ms => now;
  w => dac;
}

