<<< "rave test (poop)" >>>;

// I can get 7 here on the laptop w/ gpu (rtx 3050)
// Need to now test w/ cpu
for(int i; i < 12; i++) {
TriOsc s => Rave r => dac;
//adc => Rave r => blackhole;

// r.chan(0) => dac;
r.model(me.dir() + "rave_chafe_data_rt.ts");

330 * (i+1) => s.freq;
1 => r.gain;

}

// while(250::ms => now) <<< "output" >>>;
// 1::eon => now;
10::second => now;