//--------------------------------------------------------------------
// This is a boilerplate ChucK program generated by chuginate,
//     to help you test your new chugin, ChuGin Developer!
//--------------------------------------------------------------------
// 1) try running this program after building your chugin
//    (the bulid process should yield a AZA.chug file)
//
// 2) you can manullay load the chugin when you run this program
//    `chuck --chugin:AZA.chug AZA-test.ck`
//
// 3) OR you can put the chugin into your chugins search path
//    NOTE: not recommended until you feel the chugin to be
//    reasonably stable, as chugins in your chugins search paths
//    will automatically load every time you run `chuck` or
//    start the VM in miniAudicle...
//
// Want to see more information? Add the --verbose:3 (-v3) flag:
//    `chuck --chugin:AZA.chug AZA-test.ck -v3`
//--------------------------------------------------------------------

// print
<<< "dac has", dac.channels(), "channels" >>>;

Plateau plateau => dac;
// Noise n1 => plateau.chan(1);

<<< "plateau has", plateau.channels(), "channels" >>>;

10::samp => now;

/*
Speech API Design

Synthesis params:
- speed
- base frequency
- declination (Declination, the tendency of fundamental frequency to gradually fall over the course of an utterance)

- waveform (ignore for now, seems like speech.h has something more sophisticated)
	KW_SAW,
	KW_TRIANGLE,
	KW_SIN,
	KW_SQUARE,
	KW_PULSE,
	KW_NOISE,
	KW_WARBLE

*/

Speech speech => dac;
speech.freq(2000).rate(1.2).declination(5);

speech.say("hello world you will be assimilated") => int samples;
<<< speech.freq(), speech.rate(), speech.declination() >>>;
<<< "generated samples: ", samples >>>;
<<< "speech has ", speech.channels(), "channels" >>>;

while (true) {
    Speech s => dac;
    s.say("techno techno")::samp => now;
    .5::second => now;
}

// repeat(100) {
//     <<< speech.last() >>>;
//     samp => now;
// }

10::second => now;
