<<< "rave test (poop)" >>>;

// I can get 7 here on the laptop w/ gpu (rtx 3050)
// Need to now test w/ cpu
//repeat(1) {
TriOsc s => Rave r => dac;
//adc => Rave r => blackhole;

// r.chan(0) => dac;
r.model(me.dir() + "rave_chafe_data_rt.ts");

330 => s.freq;
1 => r.gain;

//}
1::eon => now;