@import "Chumpinate"

if (me.args() != 1) {
    <<< "Please specificy an os to package (mac, windows, linux). thx." >>>;
    me.exit();
}

me.arg(0) => string os;

if (!(os == "mac" || os == "windows" || os == "linux")) {
    <<< "Please specificy an os to package (mac, windows, linux). thx." >>>;
    me.exit();
}

// instantiate a Chumpinate package
Package pkg("HashMap");

// add our metadata here
[
    "Andrew Zhu Aday",
    "Gregg Oliva"
] => pkg.authors;

"https://github.com/ccrma/chugins/tree/hashmap/Hashmap" => pkg.homepage;
"https://github.com/ccrma/chugins/tree/hashmap/Hashmap" => pkg.repository;

"Generic HashMap + JSON parser" => pkg.description;
"MIT" => pkg.license;

["Data Structure", "Utility", "JSON", "really cool"] => pkg.keywords;

// generate a package-definition json file,
// this will be stored in (./AwesomeEffect/package.json)
"./chump" => pkg.generatePackageDefinition;

// Now we need to define a specific PackageVersion and all the associated files and metadata
PackageVersion ver("HashMap", "1.0.0");

// what is the oldest version of ChucK this package will run on?
"1.5.5.0" => ver.languageVersionMin;

// Because this is a ChucK file (and not a ChuGin, which is a complied
// binary, this package is compatible with any operating systems and
// all CPU architectures.
if (os == "mac") {
    "mac" => ver.os;
    "universal" => ver.arch;
} else {
    os => ver.os;
    "x86_64" => ver.arch;
}

// add our package's files
if (os == "windows") {
    ver.addFile("x64/Release/Hashmap.chug");
} else {
    ver.addFile("HashMap.chug");
}

// add our example, this will be stored in the package's `_examples` directory.
ver.addExampleFile("hashmap-test.ck");
ver.addExampleFile("json.json");

// zip up all our files into AwesomeEffect.zip, and tell Chumpinate what URL
// this zip file will be located at.
ver.generateVersion("./chump", "HashMap", "https://ccrma.stanford.edu/~azaday/HashMap-" + os + ".zip");

// Generate a version definition json file, stores this in "AwesomeEffect/<VerNo>/version.json"
ver.generateVersionDefinition("version", "./chump");