DbBox2D b;
int groundId, roomId, motorId;
int notes[];
ModalBar inst[0];
Pan2 pans[0];
1 => float sloMo;
5 => int maxBoxes;
0 => int numBoxes;
0 => int debug;

fun void buildAudio()
{
    [57, 58, 60, 64, 65] @=> notes;
    NHHall r => dac;

    .7 => float gain;
    inst.size(maxBoxes);
    pans.size(maxBoxes);
    for(int i;i<maxBoxes;i++)
    {
        ModalBar m @=> inst[i];
        m.preset(1); // Math.random2(0, 4));
        gain/maxBoxes => m.gain;
    
        Pan2 p @=> pans[i];
        (-1 + 2 * i/(maxBoxes-1.0)) => p.pan;
    
        m => p => r;
    }
}

fun void buildWorld()
{
    b.worldBegin(#(0, -10));
    b.newPoint(#(0, 0)) => groundId;
    b.newRoom(#(0, 10), #(10, 10), 5.0, b.dynamicType) => roomId; // xxx: allowSleep(false)

    b.newRevoluteJoint(groundId, roomId, #(0, 10), #(0, 0),
        0, /*refAngle*/
        .05 * pi, /*motorSpeed*/
        10000000. /*maxTorque*/) => motorId;
    b.worldEnd();
}

fun void addBoxAtOrigin()
{
    b.newRectangle(#(0, 10), #(.125, .125), 0, 1., b.dynamicType) => int id;
    b.setRestitution(id, Math.random2f(.5, 1.0));
}

fun void playNote(int bodyId)
{
    if(bodyId < 2) return;  /* 0 and 1 are tumbler and ground */

    bodyId - 2 => int pIndex;
    pIndex % notes.size() => int note;
    b.getPosition(bodyId) => complex p;
    b.getVelocity(bodyId) => complex v;
    -1 + (p.re + 10) / 10 => pans[pIndex].pan; // x coord is on [-10, 10]
    if(debug && bodyId == 2) // a representative body
    {
        <<<bodyId, p.re, p.im, "pan", pans[pIndex].pan()>>>;
        <<<"  v", v.re, v.im>>>;
    }
    notes[note] => Std.mtof => float f;
    f => inst[pIndex].freq;
    Math.random2f(.8, 1) => inst[pIndex].noteOn;
}

buildAudio();
buildWorld();

1::second / 60 => dur stepSize;
for(int i;i<5000;i++)
{
    b.step(stepSize);
    stepSize => now;

    if(numBoxes < maxBoxes)
    {
        addBoxAtOrigin();
        numBoxes++;
    }

    b.getNumContacts() => int c;
    if(c != 0)
    {
        // sonify the contacts
        for(int j;j<c;j++)
        {
            b.getContact(j) => vec3 info; // (bodyA, bodyB, touching)
            if(info.z != 0) // touching
            {
                playNote(info.x $ int);
                playNote(info.y $ int);
            }
        }
    }
}
b.getAvgSimTime() => dur simAvg;
<<<"average sim time", simAvg/1::second, "seconds, duty cycle", simAvg/stepSize>>>;

<<<"ringing">>>;
3::second => now;