me.dir() + "assets/clap.wav" => string file_path; // Must be absolute path.

Sampler sampler => dac;

<<< "Trying sound asset at path: ", file_path >>>;

if (sampler.read(file_path)) {
	<<< "loaded file!" >>>;
}
else {
	<<< "error reading file!" >>>;
}

<<< "num parameters: ", sampler.getNumParameters() >>>;
<<< "parameter 0 is named: ", sampler.getParameterName(0) >>>;
sampler.setParameter(0, sampler.getParameter(0));  // setting a parameter to its current value.

sampler.setParameter(0, 60./127.);  // set to center C

sampler.setParameter(1, 0); // disable Amp Active
sampler.setParameter(8, 0); // disable Filter Active

while(true) {
	60 + Std.rand2(-5, 5) => int pitch;
    sampler.noteOn(pitch, 1.);
    20::ms => now;
    sampler.noteOff(pitch, 0.);
    (Std.rand2f(20, 200))::ms => now;
}
