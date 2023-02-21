// load/unload a bunch of times to see if there's any memory leaks


fun void load() {
	SinOsc s => Rave r => blackhole;
	r.model(me.dir() + "rave_chafe_data_rt.ts");
	1::second => now;
}

while (true) {
	spork~ load();
	1::second => now;
}