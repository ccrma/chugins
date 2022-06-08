WarpBuf s1 => dac;
WarpBuf s2 => dac;

s1.gain(.3);
s2.gain(.3);

public class BPM
{
    // global variables
    static dur n1, n2, n4, n8, n16, n32, n64, n128;
    static dur n3, n6, n12;
    static dur n2d, n4d, n8d, n16d, n32d, n64d, n128d;
    
    fun void tempo(float beat) {
        // beat argument is BPM, example 120 beats per minute
        60.0/(beat) => float SPB; // seconds per beat
        
        SPB::second * 4. => n1;
        n1*0.5 => n2;
        n2*0.5 => n4;
        n4*0.5 => n8;
        n8*0.5 => n16;
        n16*0.5 => n32;
        n32*0.5 => n64;
        n64*0.5 => n128;
        
        // the dotted notes
        n2*1.5 => n2d;
        n4*1.5 => n4d;
        n8*1.5 => n8d;
        n16*1.5 => n16d;
        n32*1.5 => n32d;
        n64*1.5 => n64d;
        n128*1.5 => n128d;
        
        // the triplets
        n1 / 3. => n3;
        n1 / 6. => n6;
        n1 / 12. => n12;
        
    }
}

BPM bpm;

fun void setBPM(float newBPM) {
	bpm.tempo(newBPM);
	newBPM => s1.bpm;
	newBPM => s2.bpm;
}

setBPM(140.);

me.dir() + "assets/1375__sleep__90-bpm-nylon2.wav" => s1.read;
me.dir() + "assets/381353__waveplaysfx__drumloop-120-bpm-edm-drum-loop-022.wav" => s2.read;

fun void heartbeat() {
    while(true) {
        <<<"heartbeat">>>;
        500::ms => now;
    }
}
spork~ heartbeat();

while(true) {

	// Random transpose
	Std.rand2(-8, 8) => s2.transpose;

	// Random loop start and end
	Std.rand2(0, 24) => s1.loopStart => s1.playhead;
	s1.loopStart() + Std.rand2(1, 16) => s1.loopEnd;

	repeat(Std.rand2(1, 4)) {
		Std.rand2(0, 15) => s2.playhead;
		bpm.n32 * Std.rand2(1, 4) => now;
	}
    
}