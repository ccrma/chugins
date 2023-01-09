SinOsc sin;

Step step => Patch p => blackhole;

spork~ updateStep();

p.connect(sin, "gain");


if (p.method() != "gain") {
   <<< "FAILURE,", p.method(), "!=", "gain" >>>;
}

repeat (10) {
    <<< "gain:", sin.gain() >>>;

    if (step.next() != sin.gain()) {
       <<< "FAILURE, input" , step.next(), "!=", sin.gain() >>>;
    }
    1::samp => now;
}

fun void updateStep() {
    SinOsc lfo => blackhole;
    3 => lfo.freq;
    
    while(true) {
        (lfo.last() + 1.0) / 2.0 => step.next;
        1::samp => now;
    }
}
