1 => int numWavetables;
if (me.args()>0)
{
  me.arg(0) => Std.atoi => numWavetables;
}

<<< numWavetables >>>;
Wavetable w[numWavetables];
for (int i; i<numWavetables; i++)
{
  false => w[i].interpolate;
  w[i] => dac;
  1.0/numWavetables => w[i].gain;
  Math.random2(36,72) => Std.mtof => w[i].freq;
}

10::second => now;
