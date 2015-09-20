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
  Math.random2(36,72) => Std.mtof => w[i].freq;
}
<<< "non-interpolating" >>>;
5::second => now;

for (int i; i<numWavetables; i++)
{
  true => w[i].interpolate;
}
<<< "interpolating" >>>;
5::second => now;
