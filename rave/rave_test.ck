<<< "rave test (poop)" >>>;

// I can get 7 here on the laptop w/ gpu (rtx 3050)
// Need to now test w/ cpu
repeat(1) {
SinOsc s => Rave r => dac;
// r.chan(0) => dac;
r.model(me.dir() + "rave_chafe_data_rt.ts");

}
1::week => now;