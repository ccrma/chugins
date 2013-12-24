TriOsc foo => GVerb gverb => dac;
<<< gverb.param() >>>;
while (true)
{
  0.25 => foo.gain;
  second => now;
  0 => foo.gain;
  second => now;
}
