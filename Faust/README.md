# FaucK!!

FaucK is a Chugin allowing to combine the powerful, succinct [Functional AUdio STream (Faust) language](http://faust.grame.fr) with the strongly-timed ChucK audio programming language. FaucK allows programmers to on-the-fly evaluate Faust code directly from ChucK code and control Faust signal processors using ChucK’s sample-precise timing and concurrency mechanisms. The goal is to create an amalgam that plays to the strengths of each language, giving rise to new possibilities for rapid prototyping, interaction design and controller mapping, pedagogy, and new ways of working with both Faust and ChucK. 

**FaucK should work both in the terminal with 64-bit `chuck` and 64-bit [MiniAudicle](https://github.com/ccrma/miniAudicle).**

## Compilation and Installation

### Prerequisites for all platforms

You must install [cmake](https://cmake.org/download/) and [git](https://git-scm.com/downloads) so that they're accessible in Terminal/cmd prompts.

### Libsndfile

On Windows, the later scripts will automatically download `libsndfile`, so no action is needed.

On macOS, install the dependencies for libsndfile with `brew`:

```brew install autoconf autogen automake flac libogg libtool libvorbis opus mpg123 pkg-config```

On Linux:
```
sudo apt install autoconf autogen automake build-essential libasound2-dev \
libflac-dev libogg-dev libtool libvorbis-dev libopus-dev libmp3lame-dev \
libmpg123-dev pkg-config python
```

### Faust

If you need to update the version of Faust, you should update the Faust checkout version in `build_unix.sh`/`build_windows.bat` as well as the version number in the download scripts (`thirdparty/libfaust/download*`).

### Building FaucK

#### macOS/Linux

macOS Requirements:

* macOS version 11.0 or higher (due to Libfaust)

Navigate to `thirdparty/libfaust` and run `sh download_libfaust.sh` in Terminal.

In a Terminal window, navigate to this README and run `sh build_unix.sh`.

Navigate to the `package` directory and find the latest `fauck` version such as `fauck-0.0.1`.

On macOS, you should expect to see a `Faust.chug` directory. Copy it to `usr/local/lib/chuck`.

On Linux you should see `Faust.chug` and a `faustlibraries` sibling. Copy both as siblings to `usr/local/lib/chuck`.

Then run chuck with a FaucK example.

#### Windows

* [7z.exe](https://www.7-zip.org/download.html) must be in your system PATH.
* You must have Visual Studio 2022 installed (due to Libfaust)

Next, create a **permanent** environment variable `CHUCK_CHUGIN_PATH` and set it equal to `%USERPROFILE%/Documents/ChucK/chugins`.

Navigate to `thirdparty/libfaust`. Run `call download_libfaust.bat` in an cmd prompt.

Open an `x64 Native Tools Command Prompt for Visual Studio 2022`, navigate to this README, and run `call build_windows.bat`.

Navigate to the `package` directory and find the latest `fauck` version such as `fauck-0.0.1`. You should see a file `sndfile.dll`, which you should copy next to wherever `chuck.exe` exists on your computer (likely `C:/Program Files/ChucK/chuck.exe`). Next, copy `faustlibraries` and `Faust.chug` to `%USERPROFILE%/Documents/ChucK/chugins`.

Then run chuck with a FaucK example.

## Using FaucK

FaucK objects can be used easily in any ChucK code through a Chugin called `Faust`. For example, a new `Faust` unit generator (e.g., an audio DSP effect that takes an input from ChucK) can be declared as follow:  

```chuck
adc => Faust foo => dac;
```

In the case where `foo` would be a synthesizer, the `adc` would be ignored and we could simply write:

```chuck
Faust foo => dac;
```

Any Faust program can be associated with `foo` and dynamically evaluated by calling the `eval` method.  

```chuck
foo.eval(`process=osc(440);`);
```

For brevity and convenience, `stdfaust.lib` is by default automatically imported by FaucK. Furthermore, note the use of the backtick (\`) to delineate the inline Faust code - this removes the need to manually escape single and double quotation marks used in the Faust code.

Alternately, the same object can load a Faust program from the file system by invoking `compile` and providing a path to a Faust `.dsp` file:

```chuck
foo.compile("osc.dsp");
```

Next, the `v` method can be called at anytime to change the value of a specific parameter defined on the Faust object that is specified by its path (`v` stands for "value"; we chose this abbreviation in anticipation that most programs will invoke this method often). For example, here we create a sine wave oscillator whose only parameter is its frequency (`freq`) and we set it to 440 Hz: 

```chuck
foo.eval(`
    frequency = nentry("freq",200,50,1000,0.01);
    process = osc(frequency);
`);
foo.v("freq",440);
```

Finally, the `dump` method can be called at any time to print a list of the parameters of the Faust object as well as their current value. This is useful to observe large Faust programs that have a large number of parameters in complex grouping paths. Programmers can also directly copy the path of any parameter to control for use with the `v` method.

### Polyphony

Polyphony is supported. You simply need to provide DSP code that refers to correctly named parameters such as `freq` or `note`, `gain`, and `gate`. For more information, see the FAUST [manual](https://faustdoc.grame.fr/manual/midi/#standard-polyphony-parameters). For polyphony, you must set the number of voices to 1 or higher with the `numVoices` function. The default (0) disables polyphony. After setting the number of voices, evaluate the Faust code. Refer to `examples/polyphony-simple.ck`.

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
* `int numVoices(int value)` Get/set the number of voices. The default (0) has polyphony disabled. Set this before calling `eval`/`compile`.
* `void groupVoices(int groupVoices)` Enable/disable grouping of voices, if polyphony is enabled. The default is enabled. Not grouping voices results in having unique parameters for each voice. Set `groupVoices` before calling `eval`/`compile`.
* `void dynamicVoices(int dynamicVoices)` Enable/disable dynamic voices, if polyphony is enabled. The default is enabled. Disabling dynamic voices results in having all voices always execute, which you probably want if groupVoices is disabled. Set `dynamicVoices` before calling `eval`/`compile`.
* `void noteOn(int pitch, int velocity)` Turn on a note if polyphony is active.
* `void noteOff(int pitch, int velocity)` Turn off a note if polyphony is active.
* `void pitchWheel(int channel, int wheel)` Channel 0 means "all channels", otherwise [1-16]. `wheel` is [-8192,8192].
* `void progChange(int channel, int pgm)` Channel 0 means "all channels", otherwise [1-16]. `pgm` is [0,127].
* `void ctrlChange(int channel, int ctrl, int value)` Channel 0 means "all channels", otherwise [1-16]. `ctrl` is [0,127]. `value` is [0,127].
* `void ok()`: **todo**
* `void error()` **todo**
* `string code()` **todo**
* `void test()` **todo**

## Examples

Examples can be found in the `examples` folder of the FaucK distribution: <https://github.com/ccrma/chugins/tree/master/Faust/examples>. You can debug why the Faust.chugin may not be loading with `chuck --verbose:10 --level:10 crybaby.ck` or any other FaucK example.

## Other Resources

* Check out the 2016 SMC paper on FaucK: Ge Wang and Romain Michon, *FaucK!! Hybridizing the Faust and ChucK Audio Programming Languages*
* For other questions, feel free to e-mail Romain Michon: rmichon_AT_ccrma_DOT_stanford_DOT_edu or David Braun: braun_AT_ccrma_DOT_stanford_DOT_edu.
