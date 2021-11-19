
/* multiple VST3 plugins operating "simultaneously" ---------------*/
fun void makeSound(int moduleIndex, dur d, int verbosity)
{
    SqrOsc s => DbVST3 x => dac;
    Math.random2f(100., 200.) => s.freq;
    .1 => s.gain;
    x.setVerbosity(verbosity);
    x.loadPlugin("mda-vst3.vst3");
    x.selectModule(moduleIndex); 
    <<<moduleIndex, x.getModuleName()>>>;
    d => now;
}

// spork ~ makeSound(12, 2:second, 0); // an instrument and not a filter?
spork ~ makeSound(22, 5::second, 2);
spork ~ makeSound(22, 2::second, 0);
spork ~ makeSound(24, 8::second, 0);
spork ~ makeSound(23, 4::second, 0);
spork ~ makeSound(25, 10::second, 0); // shepard
makeSound(31, 10::second, 0);

