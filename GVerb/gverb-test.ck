adc => GVerb gverb => dac;
50 => gverb.roomsize;
2::second => gverb.revtime;
0.3 => gverb.dry;
0.2 => gverb.early;
0.5 => gverb.tail;

while (true)
{
  second => now;
}
