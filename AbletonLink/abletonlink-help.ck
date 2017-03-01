// AbletonLink produces values between 0 and resolution*quantum,
// so don't hook it up the dac!
AbletonLink link => blackhole;
2 => link.resolution; // sub-beat divisions
4 => link.quantum;
160 => link.tempo; // in bpm

// This allows me to send the beat with the event
public class LinkEvent extends Event { float beat; }

LinkEvent e;
spork~ linkListener(e); // Run AbletonLink in another thread

Shakers s => dac;
0.5 => s.gain;

spork~ resolutionChanger();

fun void resolutionChanger()
{
	while (true)
	{
		(Math.random2(5,10) * second) => now;
		Math.random2(2,5) => link.resolution;
	}

}

while (true)
{
  e => now; // Wait for a new beat from Link
  e.beat $ int => s.preset;
  1  => s.noteOn;
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
