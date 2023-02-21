<<< "rave test" >>>;
SinOsc s => Mix2 m => blackhole;


s => Rave r;

r.chan(0) => dac;
// WvOut w => blackhole;

r.model(me.dir() + "rave_chafe_data_rt.ts");
// r.method("encode");

r.help();
// m.help();
<<< r.channels() >>>;

<<< r.chan(0).channels() >>>;

// "chuck_forward.wav" => w.wavFilename;

// temporary workaround to automatically close file on remove-shred
// null @=> w;

/*
now + 5::second => time later;


while (now < later) {
	// chout <= "last: " <= r.chan(0).last() <= IO.newline() <= IO.newline();
	1::samp => now;
}
*/