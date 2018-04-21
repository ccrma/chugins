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

FaucK hasn't been tested on Windows so let us know if you make it work!

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
