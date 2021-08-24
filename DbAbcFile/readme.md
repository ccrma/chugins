# DbABCFile Chugin

## Intro

The goal of this project is to bring to ChucK an easy-to-author
text representation of musical material. The digital music landscape 
is littered with attempts to deliver something like this.  
[abcnotation](https://abcnotation.com/) is one such attempt and
has an ecosystem of supporting tools to interactively edit, preview 
and print abc scores.

DbABCFile is a parser for abc files for ChucK.  We combine the
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

`int DbABCFile.open(string filenm);`  
`int DbABCFile.close();`  
`int DbABCFile.numTracks();`  
`int DbABCFile.read(MidiMsg msg, int track);` // returns 0 when done
`int DbABCFile.rewind();`                   // returns 1 if successful

### to do

`string DbABCFile.trackInfo(int track);`   

## Example

```ck

"my.abc" => string abcfile;
DbABCFile abcIn;
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

### MidiFile anatomy

* Chunk is 4-byte id plus 4byte length
* Header and 1 or more track chunks.
* `<Header Chunk> = <chunk type><length><format><ntrks><division>`
    * <format> 16 bits:
        * 0: 1 track
        * 1: multitrack, each track is performed simultaneously
        * 2: multitrack, each track performed sequentially
    * <ntrks> 16 bits
    * <Division> - the meaning of delta-times
        * metrical-time: bit15 is 0 then we have ticks-per-quarternote.
          If division == 96, time interval of an eighth-note between 
          two events in the file would be 48.
        * timecode-time: bit15 is 1
            * bits 8-14: -24, -25, -29, -30: (fps)
            * bits 0-7: resolution in a frame: 4, 8, 10, 80 or 100.
  * `<Track Chunk> = <Track Chunk> = <chunk type><length><MTrk event>+`
    where:  `<MTrk event> = <delta-time><event>`. `<delta-time>` is stored 
    as a variable-length quantity. It represents the amount of time before 
    the _following_ event.


#### 

`AbcStore for boys.abc`

```
info: 0 LINENUM   2 0 0 0 0 0
info: 1 TITLE   0 0 0 0 0 0
info: 2 LINENUM   3 0 0 0 0 0
info: 3 TEXT   1 0 0 0 0 0
info: 4 LINENUM   4 0 0 0 0 0
info: 5 TEXT   2 0 0 0 0 0
info: 6 LINENUM   5 0 0 0 0 0
info: 7 TEXT   3 0 0 0 0 0
info: 8 LINENUM   6 0 0 0 0 0
info: 9 TEXT   4 0 0 0 0 0
info: 10 LINENUM   7 0 0 0 0 0
info: 11 LINENUM   8 0 0 0 0 0
info: 12 LINENUM   9 0 0 0 0 0
info: 13 LINENUM   10 0 0 0 0 0
info: 14 LINENUM   11 0 0 0 0 0
info: 15 VOICE   1 0 0 0 0 0   <--------------------
info: 16 LINENUM   12 0 0 0 0 0
info: 17 MUSICLINE   0 0 0 0 0 0
info: 18 DOUBLE_BAR   0 0 0 0 0 0
info: 19 BAR_REP   0 0 0 0 0 0
info: 20 NOTE   64 8192 0 1 2 2
info: 21 MUSICSTOP   0 0 0 0 0 4
info: 22 LINENUM   13 0 0 0 0 4
info: 23 MUSICLINE   0 0 0 0 0 0
info: 24 SINGLE_BAR   0 0 0 0 0 0
info: 25 NOTE   69 8192 0 1 1 2
info: 26 NOTE   64 8192 0 1 2 4
info: 27 NOTE   69 8192 0 3 4 6
info: 28 SLUR_ON   0 0 0 0 0 8
info: 29 NOTE   71 8192 0 1 4 9
info: 30 SLUR_TIE   0 0 0 0 0 9
info: 31 NOTE   72 8192 0 1 4 10
info: 32 SLUR_TIE   0 0 0 0 0 10
info: 33 NOTE   74 8192 0 1 4 12
info: 34 SLUR_TIE   0 0 0 0 0 12
info: 35 SLUR_OFF   0 0 0 0 0 14
info: 36 SINGLE_BAR   0 0 0 0 0 16
info: 37 NOTE   76 8192 0 3 4 18
info: 38 NOTE   74 8192 0 1 4 20
info: 39 NOTE   76 8192 0 1 2 21
info: 40 SLUR_ON   0 0 0 0 0 23
info: 41 NOTE   69 8192 0 1 1 24
info: 42 SLUR_TIE   0 0 0 0 0 24
info: 43 NOTE   71 8192 0 1 4 26
info: 44 SLUR_TIE   0 0 0 0 0 26
info: 45 NOTE   69 8192 0 1 4 28
info: 46 SLUR_TIE   0 0 0 0 0 28
info: 47 SLUR_OFF   0 0 0 0 0 30
info: 48 SINGLE_BAR   0 0 0 0 0 32
info: 49 NOTE   67 8192 0 1 1 34
info: 50 NOTE   62 8192 0 1 2 36
info: 51 NOTE   67 8192 0 3 4 38
info: 52 SLUR_ON   0 0 0 0 0 40
info: 53 NOTE   69 8192 0 1 4 41
info: 54 SLUR_TIE   0 0 0 0 0 41
info: 55 NOTE   71 8192 0 1 4 42
info: 56 SLUR_TIE   0 0 0 0 0 42
info: 57 NOTE   72 8192 0 1 4 44
info: 58 SLUR_TIE   0 0 0 0 0 44
info: 59 SLUR_OFF   0 0 0 0 0 46
info: 60 SINGLE_BAR   0 0 0 0 0 48
info: 61 NOTE   74 8192 0 3 4 50
info: 62 NOTE   72 8192 0 1 4 52
info: 63 NOTE   74 8192 0 1 2 53
info: 64 NOTE   67 8192 0 1 1 55
info: 65 NOTE   71 8192 0 1 2 57
info: 66 SINGLE_BAR   0 0 0 0 0 59
info: 67 MUSICSTOP   0 0 0 0 0 59
info: 68 LINENUM   14 0 0 0 0 59
info: 69 MUSICLINE   0 0 0 0 0 0
info: 70 SINGLE_BAR   0 0 0 0 0 0
info: 71 NOTE   69 8192 0 1 1 2
info: 72 NOTE   64 8192 0 1 2 4
info: 73 NOTE   69 8192 0 1 2 6
info: 74 NOTE   71 8192 0 1 2 7
info: 75 NOTE   74 8192 0 1 2 8
info: 76 SINGLE_BAR   0 0 0 0 0 10
info: 77 NOTE   76 8192 0 1 1 12
info: 78 NOTE   78 8192 0 1 2 14
info: 79 NOTE   79 8192 0 3 4 17
info: 80 NOTE   78 8192 0 1 4 19
info: 81 NOTE   79 8192 0 1 2 20
info: 82 SINGLE_BAR   0 0 0 0 0 22
info: 83 NOTE   76 8192 0 1 2 24
info: 84 NOTE   74 8192 0 1 2 25
info: 85 NOTE   72 8192 0 1 2 26
info: 86 NOTE   71 8192 0 1 2 28
info: 87 NOTE   72 8192 0 1 2 29
info: 88 NOTE   74 8192 0 1 2 30
info: 89 SINGLE_BAR   0 0 0 0 0 32
info: 90 NOTE   76 8192 0 1 2 34
info: 91 NOTE   72 8192 0 1 2 35
info: 92 NOTE   69 8192 0 1 2 36
info: 93 NOTE   69 8192 0 1 1 38
info: 94 REP_BAR   0 0 0 0 0 41
info: 95 MUSICSTOP   0 0 0 0 0 41
info: 96 LINENUM   15 0 0 0 0 41
info: 97 MUSICLINE   0 0 0 0 0 0
info: 98 BAR_REP   0 0 0 0 0 0
info: 99 NOTE   81 8192 0 1 2 3
info: 100 MUSICSTOP   0 0 0 0 0 5
info: 101 LINENUM   16 0 0 0 0 5
info: 102 MUSICLINE   0 0 0 0 0 0
info: 103 BAR_REP   0 0 0 0 0 0
info: 104 NOTE   81 8192 0 1 2 3
info: 105 NOTE   80 8192 0 1 2 4
info: 106 NOTE   81 8192 0 1 2 6
info: 107 NOTE   69 8192 0 1 1 8
info: 108 NOTE   81 8192 0 1 2 10
info: 109 SINGLE_BAR   0 0 0 0 0 12
info: 110 NOTE   81 8192 0 1 2 14
info: 111 NOTE   80 8192 0 1 2 15
info: 112 NOTE   81 8192 0 1 2 17
info: 113 NOTE   69 8192 0 1 1 19
info: 114 NOTE   80 8192 0 1 2 21
info: 115 SINGLE_BAR   0 0 0 0 0 23
info: 116 NOTE   79 8192 0 1 2 25
info: 117 NOTE   78 8192 0 1 2 26
info: 118 NOTE   79 8192 0 1 2 28
info: 119 NOTE   67 8192 0 1 1 30
info: 120 NOTE   79 8192 0 1 2 32
info: 121 SINGLE_BAR   0 0 0 0 0 35
info: 122 NOTE   79 8192 0 1 2 37
info: 123 NOTE   78 8192 0 1 2 38
info: 124 NOTE   79 8192 0 1 2 40
info: 125 NOTE   67 8192 0 1 1 42
info: 126 NOTE   71 8192 0 1 2 44
info: 127 SINGLE_BAR   0 0 0 0 0 46
info: 128 MUSICSTOP   0 0 0 0 0 46
info: 129 LINENUM   17 0 0 0 0 46
info: 130 MUSICLINE   0 0 0 0 0 0
info: 131 SINGLE_BAR   0 0 0 0 0 0
info: 132 NOTE   72 8192 0 1 1 2
info: 133 NOTE   72 8192 0 1 2 4
info: 134 NOTE   74 8192 0 1 1 6
info: 135 NOTE   74 8192 0 1 2 8
info: 136 SINGLE_BAR   0 0 0 0 0 10
info: 137 NOTE   76 8192 0 1 2 12
info: 138 NOTE   78 8192 0 1 2 13
info: 139 NOTE   79 8192 0 1 2 15
info: 140 NOTE   81 8192 0 3 2 17
info: 141 SINGLE_BAR   0 0 0 0 0 20
info: 142 NOTE   76 8192 0 1 2 22
info: 143 NOTE   74 8192 0 1 2 23
info: 144 NOTE   72 8192 0 1 2 24
info: 145 NOTE   71 8192 0 1 2 26
info: 146 NOTE   72 8192 0 1 2 27
info: 147 NOTE   74 8192 0 1 2 28
info: 148 SINGLE_BAR   0 0 0 0 0 30
info: 149 NOTE   76 8192 0 1 2 32
info: 150 NOTE   72 8192 0 1 2 33
info: 151 NOTE   69 8192 0 1 2 34
info: 152 NOTE   69 8192 0 1 1 36
info: 153 REP_BAR   0 0 0 0 0 39
info: 154 MUSICSTOP   0 0 0 0 0 39
info: 155 LINENUM   18 0 0 0 0 39
info: 156 VOICE   2 0 0 0 0 0     <-------------------------------
info: 157 LINENUM   19 0 0 0 0 0
info: 158 TIME   1 0 0 6 8 0      <------------------------------
info: 159 LINENUM   20 0 0 0 0 0
info: 160 LINENUM   21 0 0 0 0 0
info: 161 CHANNEL   9 0 0 0 0 0
info: 162 LINENUM   22 0 0 0 0 0
info: 163 MUSICLINE   0 0 0 0 0 0
info: 164 BAR_REP   0 0 0 0 0 1
info: 165 REST   0 0 0 1 2 4
info: 166 SINGLE_BAR   0 0 0 0 0 6
info: 167 NOTE   65 8192 0 1 1 7
info: 168 NOTE   65 8192 0 1 4 9
info: 169 NOTE   65 8192 0 1 4 10
info: 170 NOTE   66 8192 0 1 2 12
info: 171 NOTE   66 8192 0 1 2 15
info: 172 NOTE   66 8192 0 1 2 17
info: 173 SINGLE_BAR   0 0 0 0 0 21
info: 174 MUSICSTOP   0 0 0 0 0 21
info: 175 LINENUM   23 0 0 0 0 21
info: 176 MUSICLINE   0 0 0 0 0 0
info: 177 NOTE   65 8192 0 1 1 1
info: 178 NOTE   66 8192 0 1 2 3
info: 179 NOTE   66 8192 0 1 1 7
info: 180 NOTE   66 8192 0 1 2 10
info: 181 SINGLE_BAR   0 0 0 0 0 14
info: 182 MUSICSTOP   0 0 0 0 0 14
info: 183 LINENUM   24 0 0 0 0 14
info: 184 MUSICLINE   0 0 0 0 0 0
info: 185 NOTE   65 8192 0 1 1 1
info: 186 NOTE   65 8192 0 1 4 3
info: 187 NOTE   65 8192 0 1 4 4
info: 188 NOTE   66 8192 0 1 2 6
info: 189 NOTE   66 8192 0 1 2 9
info: 190 NOTE   66 8192 0 1 2 11
info: 191 SINGLE_BAR   0 0 0 0 0 15
info: 192 MUSICSTOP   0 0 0 0 0 15
info: 193 LINENUM   25 0 0 0 0 15
info: 194 MUSICLINE   0 0 0 0 0 0
info: 195 NOTE   65 8192 0 1 1 1
info: 196 NOTE   66 8192 0 1 2 3
info: 197 NOTE   66 8192 0 1 1 7
info: 198 NOTE   66 8192 0 1 2 10
info: 199 SINGLE_BAR   0 0 0 0 0 14
info: 200 MUSICSTOP   0 0 0 0 0 14
info: 201 LINENUM   26 0 0 0 0 14
info: 202 MUSICLINE   0 0 0 0 0 0
info: 203 NOTE   65 8192 0 1 1 1
info: 204 NOTE   65 8192 0 1 4 3
info: 205 NOTE   65 8192 0 1 4 4
info: 206 NOTE   66 8192 0 1 2 6
info: 207 NOTE   66 8192 0 1 2 9
info: 208 NOTE   66 8192 0 1 2 11
info: 209 SINGLE_BAR   0 0 0 0 0 15
info: 210 MUSICSTOP   0 0 0 0 0 15
info: 211 LINENUM   27 0 0 0 0 15
info: 212 MUSICLINE   0 0 0 0 0 0
info: 213 NOTE   65 8192 0 1 1 1
info: 214 NOTE   66 8192 0 1 2 3
info: 215 NOTE   66 8192 0 1 1 7
info: 216 NOTE   66 8192 0 1 2 10
info: 217 SINGLE_BAR   0 0 0 0 0 14
info: 218 MUSICSTOP   0 0 0 0 0 14
info: 219 LINENUM   28 0 0 0 0 14
info: 220 MUSICLINE   0 0 0 0 0 0
info: 221 NOTE   65 8192 0 1 1 1
info: 222 NOTE   65 8192 0 1 4 3
info: 223 NOTE   65 8192 0 1 4 4
info: 224 NOTE   66 8192 0 1 2 6
info: 225 NOTE   66 8192 0 1 2 9
info: 226 NOTE   66 8192 0 1 2 11
info: 227 SINGLE_BAR   0 0 0 0 0 15
info: 228 MUSICSTOP   0 0 0 0 0 15
info: 229 LINENUM   29 0 0 0 0 15
info: 230 MUSICLINE   0 0 0 0 0 0
info: 231 NOTE   65 8192 0 1 2 1
info: 232 NOTE   66 8192 0 1 2 3
info: 233 NOTE   66 8192 0 1 2 6
info: 234 NOTE   66 8192 0 1 1 9
info: 235 REP_BAR   0 0 0 0 0 13
info: 236 MUSICSTOP   0 0 0 0 0 13
info: 237 LINENUM   30 0 0 0 0 13
info: 238 MUSICLINE   0 0 0 0 0 0
info: 239 BAR_REP   0 0 0 0 0 1
info: 240 REST   0 0 0 1 2 4
info: 241 SINGLE_BAR   0 0 0 0 0 6
info: 242 NOTE   65 8192 0 1 1 7
info: 243 NOTE   65 8192 0 1 4 9
info: 244 NOTE   65 8192 0 1 4 10
info: 245 NOTE   66 8192 0 1 2 12
info: 246 NOTE   66 8192 0 1 2 15
info: 247 NOTE   66 8192 0 1 2 17
info: 248 SINGLE_BAR   0 0 0 0 0 21
info: 249 MUSICSTOP   0 0 0 0 0 21
info: 250 LINENUM   31 0 0 0 0 21
info: 251 MUSICLINE   0 0 0 0 0 0
info: 252 NOTE   65 8192 0 1 1 1
info: 253 NOTE   66 8192 0 1 2 3
info: 254 NOTE   66 8192 0 1 1 7
info: 255 NOTE   66 8192 0 1 2 10
info: 256 SINGLE_BAR   0 0 0 0 0 14
info: 257 MUSICSTOP   0 0 0 0 0 14
info: 258 LINENUM   32 0 0 0 0 14
info: 259 MUSICLINE   0 0 0 0 0 0
info: 260 NOTE   65 8192 0 1 1 1
info: 261 NOTE   65 8192 0 1 4 3
info: 262 NOTE   65 8192 0 1 4 4
info: 263 NOTE   66 8192 0 1 2 6
info: 264 NOTE   66 8192 0 1 2 9
info: 265 NOTE   66 8192 0 1 2 11
info: 266 SINGLE_BAR   0 0 0 0 0 15
info: 267 MUSICSTOP   0 0 0 0 0 15
info: 268 LINENUM   33 0 0 0 0 15
info: 269 MUSICLINE   0 0 0 0 0 0
info: 270 NOTE   65 8192 0 1 1 1
info: 271 NOTE   66 8192 0 1 2 3
info: 272 NOTE   66 8192 0 1 1 7
info: 273 NOTE   66 8192 0 1 2 10
info: 274 SINGLE_BAR   0 0 0 0 0 14
info: 275 MUSICSTOP   0 0 0 0 0 14
info: 276 LINENUM   34 0 0 0 0 14
info: 277 MUSICLINE   0 0 0 0 0 0
info: 278 NOTE   65 8192 0 1 1 1
info: 279 NOTE   65 8192 0 1 4 3
info: 280 NOTE   65 8192 0 1 4 4
info: 281 NOTE   66 8192 0 1 2 6
info: 282 NOTE   66 8192 0 1 2 9
info: 283 NOTE   66 8192 0 1 2 11
info: 284 SINGLE_BAR   0 0 0 0 0 15
info: 285 MUSICSTOP   0 0 0 0 0 15
info: 286 LINENUM   35 0 0 0 0 15
info: 287 MUSICLINE   0 0 0 0 0 0
info: 288 NOTE   65 8192 0 1 1 1
info: 289 NOTE   66 8192 0 1 2 3
info: 290 NOTE   66 8192 0 1 1 7
info: 291 NOTE   66 8192 0 1 2 10
info: 292 SINGLE_BAR   0 0 0 0 0 14
info: 293 MUSICSTOP   0 0 0 0 0 14
info: 294 LINENUM   36 0 0 0 0 14
info: 295 MUSICLINE   0 0 0 0 0 0
info: 296 NOTE   65 8192 0 1 1 1
info: 297 NOTE   65 8192 0 1 4 3
info: 298 NOTE   65 8192 0 1 4 4
info: 299 NOTE   66 8192 0 1 2 6
info: 300 NOTE   66 8192 0 1 2 9
info: 301 NOTE   66 8192 0 1 2 11
info: 302 SINGLE_BAR   0 0 0 0 0 15
info: 303 MUSICSTOP   0 0 0 0 0 15
info: 304 LINENUM   37 0 0 0 0 15
info: 305 MUSICLINE   0 0 0 0 0 0
info: 306 NOTE   65 8192 0 1 2 1
info: 307 NOTE   66 8192 0 1 2 3
info: 308 NOTE   66 8192 0 1 2 6
info: 309 NOTE   66 8192 0 1 1 9
info: 310 REP_BAR   0 0 0 0 0 13
info: 311 MUSICSTOP   0 0 0 0 0 13
info: 312 LINENUM   38 0 0 0 0 13
info: Ready to perform
note: End of File reached

writeTempo: 600000


```


`first notes from boys3.mid`:

```
---- track:1 type:144 data2:64 data3:105   (note down)
 when: 45.937500
---- track:2 type:144 data2:65 data3:105   (note down)
 when: 11070.937500
---- track:1 type:128 data2:64 data3:0     (note up)
 when: 10979.062500
---- track:1 type:144 data2:69 data3:105   (note down)
 when: 45.937500 
---- track:2 type:128 data2:65 data3:0     (note up)
 when: 22004.062500
---- track:1 type:128 data2:69 data3:0     (note up)
 when: 22004.062500
---- track:2 type:144 data2:65 data3:80   (note down)
 when: 45.937500 
---- track:1 type:144 data2:64 data3:80   (note down)
 when: 45.937500
 ```

`drmidi boys3.mid`:

```
File: boys3.mid
Format: 1
Tracks: 3
Ticks per quarter note: 480

Track chunk of 68 bytes (format1 - Track0 has timing meta-info)
000000t:  Meta-event type $01 (Text event) of length 10    
          note track
000000t:  Meta-event type $51 (Set tempo) of length 3      
          _<A1> 
000000t:  Meta-event type $59 (Key signature) of length 2  
          __
000000t:  Meta-event type $58 (Time signature) of length 4 
          __$_
000000t:  Meta-event type $03 (Track name) of length 23    
          The Boys of Carrigallen
046106t:  Meta-event type $2f (End-track event) of length 0
Sigma t = 46106

Track chunk of 1664 bytes
000000t:  Meta-event type $01 (Text event) of length 10    
          note track
000000t:  Meta-event type $03 (Track name) of length 23    
          The Boys of Carrigallen
000000t:  Meta-event type $01 (Text event) of length 15    
          B:O'Neill's 210
000000t:  Meta-event type $01 (Text event) of length 12
          N:"Cheerful"
000000t:  Meta-event type $01 (Text event) of length 26
          N:"Collected by J.O'Neill"
000000t:  Meta-event type $01 (Text event) of length 78
          Z:1997 by John Chambers <jc@eddi
          e.mit.edu> http://eddie.mit.edu/
          ~jc/music/abc/
000001t:  Note on ::  Channel 0.  Note 64.  Velocity 105
000239t:  Note off ::  Channel 0.  Note 64.  Velocity 0
000001t:  Note on ::  Channel 0.  Note 69.  Velocity 105
000479t:  Note off ::  Channel 0.  Note 69.  Velocity 0
000001t:  Note on ::  Channel 0.  Note 64.  Velocity 80
000239t:  Note off ::  Channel 0.  Note 64.  Velocity 0
000001t:  Note on ::  Channel 0.  Note 69.  Velocity 95
000359t:  Note off ::  Channel 0.  Note 69.  Velocity 0
000001t:  Note on ::  Channel 0.  Note 71.  Velocity 80
```


`from MidiFileIn`

```
This class can be used to read events from a standard MIDI file.
 Event bytes are copied to a C++ vector and must be subsequently
 interpreted by the user. The function getNextMidiEvent() skips
 meta and sysex events, returning only MIDI channel messages.
 Event delta-times are returned in the form of "ticks" and a
 function is provided to determine the current "seconds per tick".
 Tempo changes are internally tracked by the class and reflected in
 the values returned by the function getTickSeconds().
```


### issues, bugs

