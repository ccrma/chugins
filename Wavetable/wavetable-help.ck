dac => WvOut rec => blackhole;
"test" => rec.wavFilename;
1 => int numWavetables;
if (me.args()>0)
{
  me.arg(0) => Std.atoi => numWavetables;
}

Wavetable w[numWavetables];
for (int i; i<numWavetables; i++)
{
  false => w[i].interpolate;
  w[i] => dac;
  1.0/numWavetables => w[i].gain;
  Math.random2(60,72) => Std.mtof => w[i].freq;
}
<<< "non-interpolating" >>>;
2::second => now;
for (int i; i<numWavetables; i++)
{
  0 => w[i].gain;
}
10::ms => now;
for (int i; i<numWavetables; i++)
{
  1.0/numWavetables => w[i].gain;
  true => w[i].interpolate;
}
<<< "interpolating" >>>;
2::second => now;

[-1.0, -0.5, 0, 0.5, 1, 0.5, 0, -0.5, -0.9] @=> float table[];
for (int i; i<numWavetables; i++)
{
  w[i].set_table (table);
}
2::second => now;
