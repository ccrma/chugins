TriOsc foo => GVerb gverb => dac;
while (true)
{
  0.25 => foo.gain;
  second => now;
  0 => foo.gain;
  second => now;
  <<< Math.random2f(5,300) => gverb.roomsize >>>;
}
