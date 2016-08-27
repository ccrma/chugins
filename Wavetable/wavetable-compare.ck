25 => int maxOsc;
Wavetable w[1000];
SinOsc s[1000];

while (true)
{
  maxOsc++;
  <<< "Number of oscillators:",maxOsc >>>;

  <<< "SinOsc" >>>;
  for (int i; i<maxOsc; i++)
  {
    w[i] =< dac;
    Math.random2f(60,84) => Std.mtof => s[i].freq;
    1.0/maxOsc => s[i].gain;
    s[i] => dac;
  }
  5::second => now;

  <<< "Wavetable" >>>;
  for (int i; i<maxOsc; i++)
  {
    s[i] =< dac;
    Math.random2f(60,84) => Std.mtof => w[i].freq;
    1.0/maxOsc => w[i].gain;
    w[i] => dac;
    true => w[i].interpolate;
  }
  5::second => now;
}
