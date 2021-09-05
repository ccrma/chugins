# DbAbc Chugin

## Intro

The goal of this project is to bring to ChucK an easy-to-author
text representation of musical material. The digital music landscape 
is littered with attempts to deliver something like this.  
[abcnotation](https://abcnotation.com/) is one such attempt and
has an ecosystem of supporting tools to interactively edit, preview 
and print abc scores.

DbAbc is a parser for abc files for ChucK.  We combine the
design of the built-in StkMidiFileIn with the abcmidi toolset
created by James Allwright in the early 1990 and presently 
[found here](https://github.com/sshlien/abcmidi).  Many files in 
abcmidi claim GPL2 licensing status.  Guess that's a good thing 
since this source (and ChucK) is also under that scheme.

## abc2midi Divergence

Because this implementation is not targeting the _creation_ of
MIDI files, this codebase diverged significantly from the original 
system. Here, the goal is to build up a "complete" characterization of
the abc score such that it can be "performed" by repeated calls
to a `read` method.  Perhaps the most significant code divergence
is due to the re-entrancy requirements associated with the goal 
to support multiple abc files+tracks in a single ChucK process.  

Converting the 25 year-old C codebase to C++ made it easier to ensure 
that parser and store contexts were fully encapsulated. Needless to say,
this was a great deal of code-jockeying and so the current state
of affairs is far from ideal. Part of this is due to the divergence
of requirements.  Specifically, the line between AbcStore and 
AbcGenMidi is awkward and subject to reworking. There are many 1990's era
"warts" that could be removed.

## ChucK usage

In the context of ChucK, a single voice within a score can be 
performed by a single instrument in a single shred.  Following 
the design of Chuck's MidiFile UGen,  you can create multiple 
shreds to perform a multitrack score. We employ the MidiMsg datatype 
to convey the current state of the performance to the client. It's 
up to you to "act" upon the MidiMsg.  Again, following MidiFile, a 
non-zero `when` field indicates that there is nothing to do 
_at the moment_.  This value represents the relative amount of 
time to the next composition event in the requested track.

## Interface

```ck
// if pathOrString starts with X: it's a abcstring, otherwise a file.
int DbAbc.open(string pathOrString);  
int DbAbc.close();
int DbAbc.numTracks();
int DbAbc.read(MidiMsg msg, int track); // returns 0 when done
int DbAbc.rewind();                     // returns 1 if successful

# to-do
string DbAbc.trackInfo(int track);

```

## Example

```ck

"my.abc" => string abcfile;
DbAbc abcIn;
abcIn.open(abcfile) => int err;
<<< abcfile, "has", f.numTracks(), "tracks.">>>;

// for each track, spork a separate shred with the track number
int done;
for(int t; t < abcIn.numTracks(); t++)
    spork ~ doTrack(t, 1);

// keeping track of how many tracks are done
while(done < abcIn.numTracks())
    1::second => now;

// done
cherr <= "done; cleaning up..." <= IO.newline();

// close the file
abcIn.close();

// entrypoint for each shred assigned to a track
fun void doTrack(int track, float speed)
{
    Wurley s[4]; // quick polyphony
    // for each voice
    for(int i; i < s.size(); i++)
    {
        // set gain
        .3 => s[i].gain;
        // connect to output
        s[i] => reverb;
    }

    int v; // voice number for quick polyphony
    // read through all MIDI messages on track
    MidiMsg msg;
    while(abcIn.read(msg, track))
    {
        // this means no more MIDI events at current time; advance time
        if(msg.when > 0::second)
            msg.when * speed => now; // speed of 1 is nominal

        // catch NOTEON messages (lower nibble == 0x90)
        if((msg.data1 & 0xF0) == 0x90 && msg.data2 > 0 && msg.data3 > 0)
        {
            // get the pitch and convert to frequencey; set
            msg.data2 => Std.mtof => s[v].freq;
            // velocity data; note on
            msg.data3/127.0 => s[v].noteOn;
            // cycle the voices
            (v+1)%s.size() => v;

            // log
            cherr <= "NOTE ON track:" <= track <= " pitch:" <= msg.data2 <=" velocity:" <= msg.data3 <= IO.newline(); 
        }
        // other messages
        else
        {
            // log
            // cherr <= "----EVENT (unhandled) track:" <= track <= " type:" <= (msg.data1&0xF0)
            //      <= " data2:" <= msg.data2 <= " data3:" <= msg.data3 <= IO.newline(); 
        }
    }

    // done with track
    done++;
}
```

### issues, bugs, debugging

* debug by changing the verbosity level, currently in dbAbc.cpp argv
* 

