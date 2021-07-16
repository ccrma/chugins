<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;

0 => db.debug;
"../../PitchTrack/data/obama.wav" => db.read;

0 => int done;
fun void updateTriggerFreq()
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
                    db.triggerFreq() + msg.deltaX * .1 => float dfreq; 
                    if(dfreq < 5)
                        5 => dfreq;
                    else
                    if(dfreq > 200)
                        200 => dfreq;
                    <<< dfreq >>>;
                    dfreq => db.triggerFreq;
                }
            }
        }
    }
}

spork ~ updateTriggerFreq();

<<<"tiny grain as tone (synchronous granular synthesis)">>>;
.09::second => db.grainPeriod;
.43 => db.phasorStart;
.43 => db.phasorStop;
25 => db.triggerFreq; // triggers per second (varied with mouseX 5->200)
15::second => now;


<<<"tiny grain as dust (asynchronous granular synthesis)">>>;
.09::second => db.grainPeriod; // seconds per grain (long)
.43 => db.phasorStart;
.43 => db.phasorStop;
25 => db.triggerFreq; // 10 triggers per second
2 => db.triggerRange; // pct randomness per trigger


15::second => now;
1 => done;