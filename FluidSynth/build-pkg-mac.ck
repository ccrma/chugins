@import "Chumpinate"

// Our package version
"1.0.0" => string version;

<<< "Generating FluidSynth package release " >>>;

// instantiate a Chumpinate package
Package pkg("FluidSynth");

// Add our metadata...
"Spencer Salazar" => pkg.authors;

"https://github.com/ccrma/chugins" => pkg.homepage;
"https://github.com/ccrma/chugins" => pkg.repository;

"MIT" => pkg.license;
"A UGen for loading and rendering soundfont" => pkg.description;

["MIDI", "FluidSynth", "UGen"] => pkg.keywords;

// generate a package-definition.json
// This will be stored in "Chumpinate/package.json"
"./" => pkg.generatePackageDefinition;

<<< "Defining version " + version >>>;;

// Now we need to define a specific PackageVersion for test-pkg
PackageVersion ver("FluidSynth", version);

"10.2" => ver.apiVersion;

"1.5.4.0" => ver.languageVersionMin;

"mac" => ver.os;
"universal" => ver.arch;

// The chugin file
ver.addFile("./FluidSynth.chug");

// These build files are examples as well
ver.addExampleFile("FluidSynth-play.ck");
ver.addExampleFile("fluidsynth-help.ck");
ver.addExampleFile("fluidsynth-pitchbend.ck");
ver.addExampleFile("fluidsynth-tuning.ck");
ver.addExampleFile("fluidsynth-test.ck");
ver.addExampleFile("HS_African_Percussion.sf2");

// The version path
"chugins/FluidSynth/" + ver.version() + "/" + ver.os() + "/FluidSynth.zip" => string path;

<<< path >>>;

// wrap up all our files into a zip file, and tell Chumpinate what URL
// this zip file will be located at.
ver.generateVersion("./", "FluidSynth_mac", "https://ccrma.stanford.edu/~nshaheed/" + path);

chout <= "After notarizing, use the following commands to upload the package to CCRMA's servers:" <= IO.newline();
chout <= "ssh nshaheed@ccrma-gate.stanford.edu \"mkdir -p ~/Library/Web/chugins/FluidSynth/"
      <= ver.version() <= "/" <= ver.os() <= "\"" <= IO.newline();
chout <= "scp FluidSynth_mac.zip nshaheed@ccrma-gate.stanford.edu:~/Library/Web/" <= path <= IO.newline();

// Generate a version definition json file, stores this in "chumpinate/<VerNo>/FluidSynth_mac.json"
ver.generateVersionDefinition("FluidSynth_mac", "./" );