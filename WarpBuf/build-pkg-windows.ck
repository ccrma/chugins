@import "Chumpinate"

// Our package version
"0.0.1" => string version;

<<< "Generating Chumpinate package version " >>>;

// instantiate a Chumpinate package
Package pkg("WarpBuf");

// Add our metadata...
"David Braun" => pkg.authors;

"https://github.com/ccrma/chugins" => pkg.homepage;
"https://github.com/ccrma/chugins" => pkg.repository;

"MIT" => pkg.license;
"A UGen for high-quality time-stretching and pitch-shifting of audio files; also supports Ableton .asd files" => pkg.description;

[ "util", "audio processing", "time-stretching", "pitch-shifting"] => pkg.keywords;

// generate a package-definition.json
// This will be stored in "Chumpinate/package.json"
"./" => pkg.generatePackageDefinition;

<<< "Defining version " + version >>>;;

// Now we need to define a specific PackageVersion for test-pkg
PackageVersion ver("WarpBuf", version);

"10.2" => ver.apiVersion;

"1.5.4.0" => ver.languageVersionMin;

"windows" => ver.os;
"x86_64" => ver.arch;

// The chugin file
ver.addFile("build/Release/WarpBuf.chug");

// Our deps
ver.addDepsFile("thirdparty/libsndfile-1.2.0-win64/bin/sndfile.dll");

// These build files are examples as well
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
"chugins/WarpBuf/" + ver.version() + "/" + ver.os() + "/WarpBuf.zip" => string path;

<<< path >>>;

// wrap up all our files into a zip file, and tell Chumpinate what URL
// this zip file will be located at.
ver.generateVersion("./", "WarpBuf_win", "https://ccrma.stanford.edu/~nshaheed/" + path);

chout <= "Use the following commands to upload the package to CCRMA's servers:" <= IO.newline();
chout <= "ssh nshaheed@ccrma-gate.stanford.edu \"mkdir -p ~/Library/Web/chugins/WarpBuf/"
      <= ver.version() <= "/" <= ver.os() <= "\"" <= IO.newline();
chout <= "scp WarpBuf_win.zip nshaheed@ccrma-gate.stanford.edu:~/Library/Web/" <= path <= IO.newline();

// Generate a version definition json file, stores this in "chumpinate/<VerNo>/Chumpinate_win.json"
ver.generateVersionDefinition("WarpBuf_win", "./" );