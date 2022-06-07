# FaucK!!

FaucK is a Chugin allowing to combine the powerful, succinct [Functional AUdio STream (Faust) language](http://faust.grame.fr) with the strongly-timed ChucK audio programming language.
FaucK allows programmers to on-the-fly evaluate Faust code directly from ChucK code and control Faust signal processors using ChucK’s sample-precise timing and
concurrency mechanisms. The goal is to create an amalgam that plays to the strengths of each language, giving rise to new possibilities for rapid prototyping, interaction design and controller mapping, pedagogy, and new ways of working with both Faust and ChucK. 

**FaucK should work both in the terminal with 64-bit `chuck` and 64-bit MiniAudicle.**

## Compilation and Installation

### Faust

FaucK uses the "on-the-fly" LLVM based version of the Faust compiler (<http://faust.grame.fr>) also known as `libfaust`. Luckily, we've already compiled `libfaust` for you on various platforms (see [TD-Faust](https://github.com/DBraun/TD-Faust/) for more info). However, we still need the [Faust Libraries](https://faustlibraries.grame.fr/). If on macOS/Linux, [download the libraries](https://github.com/grame-cncm/faustlibraries/archive/refs/heads/master.zip) to `/usr/local/share/faust/`. For example, this should result in the existence of this file: `/usr/local/share/faust/all.lib`. If on Windows, [download the libraries](https://github.com/grame-cncm/faustlibraries/archive/refs/heads/master.zip) to `C:/Program Files (x86)/ChucK/share/faust` or wherever your ChucK is installed. What's important is that the `share` folder is next to the `bin` folder which contains `chuck.exe`.

Next, download [Faust's source code](https://github.com/grame-cncm/faust) to a directory.

### Building FaucK

#### macOS/Linux

##### Code-signing

> On macOS, you must codesign the Faust.chug. To prepare to do this, you will set an environment variable. Find your Development Profile. Open Keychain Access, go to 'login' on the left, and look for something like Apple Development: example@example.com (ABCDE12345). Then in Terminal, run `export CODESIGN_IDENTITY="Apple Development: example@example.com (ABCDE12345)"` with your own info substituted. If you weren't able to find your profile, you need to create one. Open Xcode, go to "Accounts", add your Apple ID, click "Manage Certificates", and use the plus icon to add a profile. Then check Keychain Access again.

Create an environment variable `FAUST_DIR` which contains [Faust](https://github.com/grame-cncm/faust). For example, 

```bash
export FAUST_DIR=/some/path/to/faust
```

Download [Libsndfile](https://github.com/libsndfile/libsndfile) to a directory. Navigate to it in a Terminal/shell window. Then,
```bash
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE=ON -DENABLE_EXTERNAL_LIBS=off -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build --config Release
```

Create an environment variable `SndFile_DIR` which contains the new `build` folder, for example

```bash
export SndFile_DIR=/some/path/to/libsndfile/build
```

In a Terminal/shell window in which `FAUST_DIR` and `SndFile_DIR` have been set, navigate to this README and run either `sh build_macos.sh` or `sh build_linux.sh`.

On macOS/Linux, the goal is to create two files inside `/usr/local/lib/chuck`:

* `Faust.chug`
* `libfaust.2.dylib`

so check that these files exist. Then run chuck with a FaucK example.

#### Windows

Go to your Documents folder: `%USERPROFILE%/Documents`. Inside, create a `ChucK` folder. Inside this folder, create a `chugins` folder.

Next, create a **permanent** environment variable `CHUCK_CHUGIN_PATH` and set it equal to `%USERPROFILE%/Documents/ChucK/chugins`.

Navigate to this README and run `call build_windows.bat`.

On Windows, the goal is to create `Faust.chug` inside `%USERPROFILE%/Documents/ChucK/chugins` and `faust.dll` and `sndfile.dll` inside `C:/Program Files (x86)/ChucK/bin/`, so check that these files exist. Then run chuck.exe with a FaucK example.

## Using FaucK

FaucK objects can be used easily in any ChucK code through a Chugin called `Faust`. For example, a new `Faust` unit generator (e.g., an audio DSP effect that takes an input from ChucK) can be declared as follow:  

```
adc => Faust foo => dac;
```

In the case where `foo` would be a synthesizer, the `adc` would be ignored and we could simply write:

```
Faust foo => dac;
```

Any Faust program can be associated with `foo` and dynamically evaluated by calling the `eval` method.  

```
foo.eval(`process=osc(440);`);
```

For brevity and convenience, `stdfaust.lib` is by default automatically imported by FaucK.  Furthermore, note the use of the backtick (\`) to delineate the inline Faust code - this removes the need to manually escape single and double quotation marks used in the Faust code.

Alternately, the same object can load a Faust program from the file system by invoking `compile` and providing a path to a Faust `.dsp` file:

```
foo.compile("osc.dsp");
```

Next, the `v` method can be called at anytime to change the value of a specific parameter defined on the Faust object that is specified by its path (`v` stands for "value"; we chose this abbreviation in anticipation that most program will invoke this method often). For example, here we create a sine wave oscillator whose only parameter is its frequency (`freq`) and we set it to 440 Hz: 

```
foo.eval(`
    frequency = nentry("freq",200,50,1000,0.01);
    process = osc(frequency);
`);
foo.v("freq",440);
```

Finally, the `dump` method can be called at any time to print a list of the parameters of the Faust object as well as their current value.  This is useful to observe large Faust programs that have a large number of parameters in complex grouping paths. Programmers can also directly copy the path of any parameter to control for use with the `v` method.

### Polyphony

Polyphony is supported. You simply need to provide DSP code that refers to correctly named parameters such as `freq` or `note`, `gain`, and `gate`. For more information, see the FAUST [manual](https://faustdoc.grame.fr/manual/midi/#standard-polyphony-parameters). For polyphony, you must set the number of voices to 1 or higher with the `numVoices` function. The default (0) disables polyphony. Refer to `examples/polyphony-simple.ck`.

### Full API

A Faust Chugin has the following functions:
* `void dump()` Print out information about the Faust parameters in the ChucK console.
* `void v(string name, float value)` Set a value for a named parameter.
* `float v(string name)` Get a value for a named parameter.
* `void panic()` Turn off all voices if polyphony is active.
* `void eval(string code)` Evaluate a string of Faust code.
* `void compile(string filepath)` Load and evaluate Faust code from a file path.
* `void assetsDir(string dirpath)` Set the directory containing soundfiles which you want Faust to load.
* `void librariesDir(string dirpath)` Set the directory containing your custom Faust `.lib` files.
* `int numVoices(int value)` Get/set the number of voices. The default (0) has polyphony disabled.
* `void groupVoices(int groupVoices)` Enable/disable grouping of voices, if polyphony is enabled. Not grouping voices results in having unique parameters for each voice.
* `void noteOn(int pitch, int velocity)` Turn on a note if polyphony is active.
* `void noteOff(int pitch, int velocity)` Turn off a note if polyphony is active.
* `void pitchWheel(int pitch, int wheel)`
* `void progChange(int channel, int pgm)`
* `void ctrlChange(int channel, int ctrl, int value)`
* `void ok()`: **todo**
* `void error()` **todo**
* `void code()` **todo**
* `void test()` **todo**

## Examples

Examples can be found in the `examples` folder of the FaucK distribution: <https://github.com/ccrma/chugins/tree/master/Faust/examples>. You can debug why the Faust.chugin may not be loading with `chuck --verbose:10 --level:10 crybaby.ck` or any other FaucK example.

## Other Resources

* Check out the 2016 SMC paper on FaucK: Ge Wang and Romain Michon, *FaucK!! Hybridizing the Faust and ChucK Audio Programming Languages*
* For other questions, feel free to e-mail Romain Michon: rmichon_AT_ccrma_DOT_stanford_DOT_edu or David Braun: braun_AT_ccrma_DOT_stanford_DOT_edu.
