@import "Chumpinate"

// Our package version
"1.0.0" => string version;

<<< "Generating Chumpinate package version " >>>;

// instantiate a Chumpinate package
Package pkg("Line");

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
PackageVersion ver("Line", version);

"10.2" => ver.apiVersion;

"1.5.4.0" => ver.languageVersionMin;

"windows" => ver.os;
"x86_64" => ver.arch;

// The chugin file
ver.addFile("./x64/Release/Line.chug");

// These build files are examples as well
ver.addExampleFile("examples/basic.ck");
ver.addExampleFile("examples/tremolo.ck");
ver.addExampleFile("examples/multi_ramp.ck");

// The version path
"chugins/Line/" + ver.version() + "/" + ver.os() + "/Line.zip" => string path;

<<< path >>>;

// wrap up all our files into a zip file, and tell Chumpinate what URL
// this zip file will be located at.
ver.generateVersion("./", "Line_windows", "https://ccrma.stanford.edu/~nshaheed/" + path);

chout <= "Use the following commands to upload the package to CCRMA's servers:" <= IO.newline();
chout <= "ssh nshaheed@ccrma-gate.stanford.edu \"mkdir -p ~/Library/Web/chugins/Line/"
      <= ver.version() <= "/" <= ver.os() <= "\"" <= IO.newline();
chout <= "scp Line_windows.zip nshaheed@ccrma-gate.stanford.edu:~/Library/Web/" <= path <= IO.newline();

// Generate a version definition json file, stores this in "chumpinate/<VerNo>/Line_windows.json"
ver.generateVersionDefinition("Line_windows", "./" );
