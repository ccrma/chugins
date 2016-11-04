# FaucK!!

FaucK is a Chugin allowing to combine the powerful, succinct [Functional AUdio STream (Faust) language](http://faust.grame.fr) with the strongly-timed ChucK audio programming language.
FaucK allows programmers to on-the-fly evaluate Faust code directly from ChucK code and control Faust signal processors using ChucK's sample-precise timing and
concurrency mechanisms. The goal is to create an amalgam that plays to the strengths of each language, giving rise to new possibilities for rapid prototyping, interaction design and controller mapping, pedagogy, and new ways of working with both Faust and ChucK. 

FaucK should work both in the terminal and MiniAudicle.

**Warning:** FaucK is still being developed and things are moving quickly. Use it a your own risks!

## Known Issues/Bugs (Better Start with That ;) )

* Currently, FaucK has to be built with a predefined maximum number of inputs and outputs. This is a limitation of the Chugin architecture. By default, Fauck is built to have a maximum of 8 inputs and 8 outputs. Of course, only channels that are being used are actually computed. To build FaucK with a custom version of inputs and outputs, please refer to the build section below.
* If MiniAudicle crashes when you start its virtual machine, try to edit its preferences, restart the software and start the virtual machine again. This sounds very hacky, but this bug has been reported by some beta testers and we were not able to reproduce it on our own computer. This workaround seems to work though :).
* Beside that, there are probably dozens of bugs that we don't know yet: it's your job to tell us about them :).

## Installing FaucK

### OSX

Unfortunately, FaucK is only compatible with versions of OSX greater or equal to 10.9 (thanks Apple).

There is an installation package including FaucK, MiniAudicle and Chuck available on OSX that can be downloaded [here](https://ccrma.stanford.edu/~rmichon/fauck/stuff/chuck-1.3.6.0-220a-1.pkg). It is still being beta tested. Also, it might not be completely up-to-date.

Alternatively, you can use [this build](https://ccrma.stanford.edu/~rmichon/fauck/stuff/fauckOSX.zip) which should contain the latest version (it was built on Nov. 4, 2016) of FaucK, MiniAudicle and Chuck (read the README to know how to use it).

Finally, building FaucK from scratch might not be a bad idea... If you want to do that, please refer to the following section.

### Linux

FaucK is still being actively developed, therefore it hasn't been packaged for any Linux distribution yet. If you wish to use FaucK on Linux, you will have to build it from scratch (see the following section on building FaucK).

### Windows

Hahahahaha!

## Building FaucK (Unix Systems Only)

First, you should compile Chuck and MiniAudicle using the `fauck` branch for both cases which should be done like this (more or less):

```
git clone https://github.com/ccrma/chuck.git
cd chuck
git checkout fauck
cd src
make [yourTarget]
sudo make install
```

```
git clone https://github.com/ccrma/miniAudicle
cd miniAudicle
git checkout fauck
cd src
make [yourTarget]
sudo make install
```

FaucK uses the "on-the-fly" LLVM based version of the Faust compiler (<http://faust.grame.fr>) also known as `libfaust`. Thus, in order to compile FaucK, you need to make sure that `faust2` is installed on your system. `faust2` is an experimental branch of the Faust compiler, so it is very unlikely that you wont find a pre-compiled package for it. The latest stable release of the source code of `faust2` can be downloaded [here](https://sourceforge.net/projects/faudiostream/files/), however we recommend you to get a snapshot of the `faust2` git repository: <https://sourceforge.net/p/faudiostream/code/ci/faust2/tree/>. `faust2` has a couple of dependencies so you might have to fix them before you can do anything. For instance, the `LLVM` developement files (we recommend version 3.8 or <) as well as `openssl` will be needed.

Once you compiled and installed `faust2`, you should be able to build FaucK. For this, you must first get the latest version of the `Chugins` source code:

```
git clone https://github.com/ccrma/chugins.git
cd chugins
```

Next, switch to the `fauck` branch:

```
git checkout fauck
```

If you only want to build FaucK, go in the Faust folder (`cd Faust`), otherwise, follow the regular build instructions contained in the `Chugins` README.

To build FaucK on Linux, just type:

```
make linux
sudo make install
```

on OSX:

```
make osx
sudo make install
```

As this was explained previously, FaucK has to be compiled with a maximum number of inputs and outputs. The default is 12x12 but you can change that by using the `MAX_INPUTS` and  `MAX_OUTPUTS` compilation options. For example, if you want FaucK to have a maximum of 2 inputs and 4 outputs on Linux you would run something like:

```
make linux MAX_INPUTS=2 MAX_OUTPUTS=4
```

Hopefully, everything went well and you're now ready to FaucK around! If it's not the case, shoot me an e-mail!

### Windows

FaucK hasn't been tested on Windows so let us know if you make it work! Good luck...

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
foo.eval(`process=os.osc(440);`);
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

* Check out the 2016 SMC paper on FaucK: Ge Wang and Romain Michon, [FaucK!! Hybridizing the Faust and ChucK Audio Programming Languages](https://ccrma.stanford.edu/~rmichon/publications/doc/SMC16-fauck.pdf)
* For other questions, feel free to e-mail Romain Michon: rmichon_AT_ccrma_DOT_stanford_DOT_edu
