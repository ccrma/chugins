<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;

0 => db.debug;
"../../PitchTrack/data/obama.wav" => db.read;

0 => int done;
fun void updateGrainPeriod(dur min, dur max)
{
    Hid hi;
    HidMsg msg;
    // open mouse 0, exit on fail
    if( !hi.openMouse(0) ) me.exit();
    <<< "mouse '" + hi.name() + "' ready", "" >>>;
    while(!done)
    {
        hi => now;
        while(hi.recv(msg))
        {
            if(msg.isMouseMotion())
            {
                if(msg.deltaX)
                {
                    (msg.deltaX * .01)::second => dur dPeriod;
                    db.grainPeriod() + dPeriod => dur p; 
                    if(p < min)
                        min => p;
                    else
                    if(p > max)
                        max => p;
                    <<< "grainPeriod", p >>>;
                    p => db.grainPeriod;
                }
            }
        }
    }
}

spork ~ updateGrainPeriod(.01::second, 4.::second);

<<<"varying grain size (synchronous)">>>;
.09::second => db.grainPeriod; // seconds per grain (long)
.43 => db.phasorStart;
.43 => db.phasorStop;
10 => db.triggerFreq; // triggers per second (varied with mouseX 5->200)
.5 => db.gain; // as grainPeriod increases more overlap => louder
15::second => now;

<<<"lower trigger">>>;
.09::second => db.grainPeriod; // seconds per grain (long)
.43 => db.phasorStart;
.43 => db.phasorStop;
4 => db.triggerFreq; // triggers per second (varied with mouseX 5->200)
.5 => db.gain; // as grainPeriod increases more overlap => louder
15::second => now;


