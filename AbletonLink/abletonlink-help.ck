// AbletonLink produces values between 0 and resolution*quantum,
// so don't hook it up the dac!
AbletonLink link => blackhole;
1 => link.resolution; // sub-beat divisions
4 => link.quantum;
160 => link.tempo; // in bpm

// This allows me to send the beat with the event
public class LinkEvent extends Event { float beat; }

LinkEvent e;
spork~ linkListener(e); // Run AbletonLink in another thread

SinOsc s => dac;
0.25 => s.gain;

while (true)
{
  e => now; // Wait for a new beat from Link
  (e.beat+2) * 110 => s.freq; // Use that beat value to change pitch
}

fun void linkListener (LinkEvent e)
{
  -1 => float last;
  while (true)
  {
    if (link.last() != last) // that is, if the beat counter advances
    {
      link.last() => last;
      last => e.beat;
      e.broadcast();
    }
    samp => now;
  }
}
