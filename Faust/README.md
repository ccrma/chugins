# FaucK!!

FaucK is a Chugin allowing to combine the powerful, succinct [Functional AUdio STream (Faust) language](http://faust.grame.fr) with the strongly-timed ChucK audio programming language.
FaucK allows programmers to on-the-fly evaluate Faust code directly from ChucK code and control Faust signal processors using ChucK’s sample-precise timing and
concurrency mechanisms. The goal is to create an amalgam that plays to the strengths of each language, giving rise to new possibilities for rapid prototyping, interaction design and controller mapping, pedagogy, and new ways of working with both Faust and ChucK. 

FaucK should work both in the terminal and MiniAudicle (at least on Linux for the latest). 

## Compilation and Installation

### OSX and Linux

FaucK uses the "on-the-fly" LLVM based version of the Faust compiler (<http://faust.grame.fr>) also known as `libfaust`. Thus, in order to compile FaucK, Faust should be installed on your system. If you don't have it, get it from the Faust Git Repo (<https://github.com/grame-cncm/faust>). `faust` has a couple of dependencies so you might have to fix them before you can do anything. For instance, the `LLVM` developement files (we recommend version 3.8 or <) as well as `openssl` will be needed.

Once you compiled and installed `faust`, you should be able to build FaucK. On Linux, just type:

```
make linux
sudo make install
```

on OSX:

```
make osx
sudo make install
```

Hopefully, everything went well and you're now ready to FaucK around!

### Windows

#### Overview

These instructions will build a 64-bit Faust chugin. Therefore, it is **incompatible** with 32-bit `miniAudicle` and 32-bit `chuck.exe`.

#### Preparation

Go to your Documents folder: `%USERPROFILE%/Documents`. Inside, create a `ChucK` folder. Inside this folder, create a `chugins` folder.

Next, create a permanent environment variable `CHUCK_CHUGIN_PATH` and set it equal to `%USERPROFILE%/Documents/ChucK/chugins`.

#### Install FAUST

Download and use the latest `win64.exe` installer for [FAUST](https://github.com/grame-cncm/faust/releases).

Copy the `.lib` files in `C:/Program Files/Faust/share/faust/` to `C:/Program Files (x86)/ChucK/share/faust` or wherever your ChucK is installed. What's important is that the `share` folder is next to the `bin` folder which contains `chuck.exe`.

#### Using Prebuilt Everything

To avoid compiling everything yourself, you may simply follow the instructions in this subsection.

Copy `win-x64/Release/Faust.chug` to `%USERPROFILE%/Documents/ChucK/chugins`. Copy `win-x64/Release/faust.dll` to `C:/Program Files (x86)/ChucK/bin`.

#### Building LLVM

The remaining instructions involve cloning repositories to `C:/repos`. You can make changes if you want your repositories saved elsewhere.

Clone the [LLVM Project](https://github.com/llvm/llvm-project) repository:
`git clone https://github.com/llvm/llvm-project C:/repos/llvm-project`

Configure with CMake:
```bash
cd C:/repos/llvm-project/llvm
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -DLLVM_USE_CRT_DEBUG=MDd -DLLVM_USE_CRT_RELEASE=MD -DLLVM_BUILD_TESTS=Off -DCMAKE_INSTALL_PREFIX="./llvm" -Thost=x64
```

(Note that the `MD` flags build a dynamic library, whereas `MT` would have built a static library.)

Then open `C:/repos/llvm-project/llvm/build/LLVM.sln` and build in Release/64. This will take at least 20 minutes.

#### Building libfaust

Clone the [FAUST](https://github.com/grame-cncm/faust/) repository:
`git clone https://github.com/grame-cncm/faust.git C:/repos/faust`

Libfaust is compiled as a step during the compilation of the Faust chugin, which comes next.

#### Building Faust.chug

Open a cmd window to the directory containing this README.

Configure with CMake:
```bash
mkdir build
cd build
set LLVM_DIR=C:/repos/llvm-project/llvm/build/lib/cmake/llvm
cmake .. -DUSE_LLVM_CONFIG=off -DFAUST_DIR=C:/repos/faust
```

Then open `FaucK.sln` and build in Release/64.

#### Post-build

Confirm that `%USERPROFILE%/Documents/ChucK/chugins` contains `Faust.chug`.

Copy `C:/repos/faust/build/lib/Release/faust.dll` (about 30 MB) to `C:/Program Files (x86)/ChucK/bin/faust.dll`.

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

Next, the `v` method can be called at anytime to change the value of a specific parameter defined on the Faust object that is specified by its path (`v` stands for "value"; we chose this abbreviation in anticipation that most program will invoke this method often). For example, here we create a sine wave oscillator whose only parameter is its frequency (`freq`) and we set it to 440Hz: 

```
foo.eval(`
    frequency = nentry("freq",200,50,1000,0.01);
    process = osc(frequency);
`);
foo.v("freq",440);
```

Finally, the `dump` method can be called at any time to print a list of the parameters of the Faust object as well as their current value.  This is useful to observe large Faust programs that have a large number of parameters in complex grouping paths. Programmers can also directly copy the path of any parameter to control for use with the `v` method.

## Examples

Examples can be found in the `examples` folder of the FaucK distribution: <https://github.com/ccrma/chugins/tree/master/Faust/examples>

## Other Resources

* Check out the 2016 SMC paper on FaucK: Ge Wang and Romain Michon, *FaucK!! Hybridizing the Faust and ChucK Audio Programming Languages*
* For other questions, feel free to e-mail Romain Michon: rmichon_AT_ccrma_DOT_stanford_DOT_edu
