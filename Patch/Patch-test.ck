SinOsc sin;

// Patch.help();

Patch p => blackhole;
// sin => Patch p => blackhole;
// SinOsc s;

// <<< "calling method", p.method() >>>;



p.connect(sin);

repeat (10) {
        <<< sin.gain() >>>;
	1::samp => now;
}