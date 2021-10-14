
/* multiple VST3 plugins operating "simultaneously" ---------------*/
fun void makeSound(int moduleIndex, dur d)
{
    SqrOsc s => DbVST3 x => dac;
    Math.random2f(100., 200.) => s.freq;
    .1 => s.gain;
    x.loadPlugin("mda-vst3.vst3");
    x.selectModule(moduleIndex); 
    <<<moduleIndex, x.getModuleName()>>>;
    d => now;
}

// spork ~ makeSound(12); // an instrument and not a filter?
spork ~ makeSound(22, 5::second);
spork ~ makeSound(22, 2::second);
spork ~ makeSound(24, 8::second);
spork ~ makeSound(23, 4::second);
spork ~ makeSound(25, 10::second); // shepard
makeSound(31, 10::second);

