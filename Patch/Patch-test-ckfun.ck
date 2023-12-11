class Foo {
  fun void run(float f) {
  
  }

  fun void run2(float f, float q) {

  }

  fun float run3(float f) { return f; }

  fun float run4(int f) { return f; }
}

Step step => Patch p => blackhole;
Foo foo;
<<< foo >>>;

spork~ updateStep();

p.connect(foo, "run4");
<<< foo >>>;

if (p.method() != "run4") {
   <<< "FAILURE,", p.method(), "!=", "run" >>>;
}

repeat (10) {
    // <<< step.next(), foo.gain() >>>;
    // if (step.next() != sin.gain()) {
    //    <<< "FAILURE, input" , step.next(), "!=", sin.gain() >>>;
    // }
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
