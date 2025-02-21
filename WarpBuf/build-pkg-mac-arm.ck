@import "Chumpinate"

// Our package version
"0.0.1" => string version;

<<< "Generating Chumpinate package version " >>>;

// instantiate a Chumpinate package
Package pkg("WarpBuf");

// Add our metadata...
"Nick Shaheeed" => pkg.authors;

"https://github.com/ccrma/chugins" => pkg.homepage;
"https://github.com/ccrma/chugins" => pkg.repository;

"MIT" => pkg.license;
"Envelope of a arbitrary ramps (ala Max/PD's line~ object)" => pkg.description;

["util", "envs", "envelopes"] => pkg.keywords;

// generate a package-definition.json
// This will be stored in "Chumpinate/package.json"
"./" => pkg.generatePackageDefinition;

<<< "Defining version " + version >>>;;

// Now we need to define a specific PackageVersion for test-pkg
PackageVersion ver("WarpBuf", version);

"10.2" => ver.apiVersion;

"1.5.4.0" => ver.languageVersionMin;

"mac" => ver.os;
"arm64" => ver.arch;

// The chugin file
ver.addFile("./WarpBuf.chug");

// These build files are examples as well
// ver.addExampleFile("examples/basic.ck");
// ver.addExampleFile("examples/tremolo.ck");
// ver.addExampleFile("examples/multi_ramp.ck");
ver.addExampleFile("tests/warpbuf_no_warp_file.ck");
ver.addExampleFile("tests/warpbuf_basic.ck");
ver.addExampleFile("tests/warpbuf_silent.ck");
ver.addExampleFile("tests/warpbuf_advanced.ck");
ver.addExampleFile("tests/assets/381353__waveplaysfx__drumloop-120-bpm-edm-drum-loop-022_no_asd.wav", "assets");
ver.addExampleFile("tests/assets/1375__sleep__90-bpm-nylon2.wav.asd", "assets");
ver.addExampleFile("tests/assets/1375__sleep__90-bpm-nylon2.wav", "assets");
ver.addExampleFile("tests/assets/381353__waveplaysfx__drumloop-120-bpm-edm-drum-loop-022.wav.asd", "assets");
ver.addExampleFile("tests/assets/381353__waveplaysfx__drumloop-120-bpm-edm-drum-loop-022.wav", "assets");

// The version path
"chugins/WarpBuf/" + ver.version() + "/" + ver.os() + "/WarpBuf_arm64.zip" => string path;

<<< path >>>;

// wrap up all our files into a zip file, and tell Chumpinate what URL
// this zip file will be located at.
ver.generateVersion("./", "WarpBuf_mac_arm64", "https://ccrma.stanford.edu/~nshaheed/" + path);

chout <= "Use the following commands to upload the package to CCRMA's servers:" <= IO.newline();
chout <= "ssh nshaheed@ccrma-gate.stanford.edu \"mkdir -p ~/Library/Web/chugins/WarpBuf/"
      <= ver.version() <= "/" <= ver.os() <= "\"" <= IO.newline();
chout <= "scp WarpBuf_mac_arm64.zip nshaheed@ccrma-gate.stanford.edu:~/Library/Web/" <= path <= IO.newline();

// Generate a version definition json file, stores this in "chumpinate/<VerNo>/Chumpinate_mac_arm64.json"
ver.generateVersionDefinition("WarpBuf_mac_arm64", "./" );