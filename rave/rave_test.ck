<<< "rave test" >>>;
// SinOsc s => Mix2 m => blackhole;

/* problems and stuffs with GPU

- fucks up on second call? after making a compute thread?

*/

//repeat(1) {
adc => Rave r1;
r1.chan(0) => dac;
r1.model(me.dir() + "rave_chafe_data_rt.ts");
//}

/*
// 1024::samp => now; 
adc => Rave r2;
r2.chan(0) => dac;
r2.model(me.dir() + "rave_chafe_data_rt.ts");

2048::samp => now;
*/


while(true) {
1::samp => now;
// <<< r1.chan(0).last() >>>;
}

// r.method("encode");

// r.help();
// m.help();
// <<< r.channels() >>>;

// <<< r.chan(0).channels() >>>;

// "chuck_forward.wav" => w.wavFilename;

// temporary workaround to automatically close file on remove-shred
// null @=> w;

/*
// now + 5::second => time later;

1::hour => now;
while (true) {
	// chout <= "last: " <= r.chan(0).last() <= IO.newline() <= IO.newline();
	1::samp => now;
}
*/