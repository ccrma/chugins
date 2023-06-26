# Repository for Officially Supported ChuGins
A _chugin_ (or ChuGin) is a plug-in for the [ChucK audio programming language](https://github.com/ccrma/chuck). Chugins are loaded at runtime by [ChucK](https://github.com/ccrma/chuck) or [miniAudicle](https://github.com/ccrma/miniAudicle) and are fundmental to extending the capabilities of the language by adding class libraries and audio Unit Generators (UGens) and Unit Analyzers (UAnae). Once loaded, the contents of chugins can be used directly from the language. This repository contains a number of chugins maintained for all supported platforms (macOS, Linux, Windows) by the ChucK Team in collaboration with their respective authors (see list below). The ChucK Team invites developers to envision and create new chugins, and to help maintain and evolve existing ones. Join in on the [ChucK Discord Server](https://discord.gg/ENr3nurrx8), part of the [ChucK Community](https://chuck.stanford.edu/community/).

## Prepackaged Binaries
Binaries for most chugins in this repository are automatically included by the ChucK installers for macOS and Windows. Install the latest at [http://chuck.stanford.edu/release/](http://chuck.stanford.edu/release/). You can read the [class library reference](https://chuck.stanford.edu/doc/reference/chugins.html) for these packaged chugins. Linux users (and advanced users on macOS and Windows) can build chugins from source.

## Building ChuGins
Clone this repository:
```
git clone --recurse-submodules https://github.com/ccrma/chugins.git
cd chugins
```
To compile all chugins in this repository, run `make linux` (Linux) or `make osx` (macOS). Windows users can compile using the Visual Studio solution `chugins.sln`.

Optionally, (Linux and macOS, requires administrative privileges) install the chugins:
```
sudo make install
```

## Working with Chugins
Chugins are loaded automatically by command-line **chuck**, or when **miniAudicle** starts the ChucK VM, and can be used directly from the language (see [class library documentation](https://chuck.stanford.edu/doc/reference/chugins.html) for the pre-packaged chugins).

For a chugin to load properly:
1. the chugin version must be compatible with the host version
2. the chugin must be located somewhere within one of the default search directories, or be explicitly specified, e.g., using the `--chugin:<file>` flag on the command line.

To see which chugins are being loaded on your system and their versions, do one of the following.

If you are using command-line **chuck**, use the `--chugin-probe` flag (need chuck-1.5.0.4 or higher) to see a diagnostic of chugins on your system. (Note this option does not actually run any chuck programs.)
```
chuck --chugin-probe
```
You can increase the amount of information by additionally specifying the verbosity with `--verbose` or simply `-v`:
```
chuck --chugin-probe -v5
```
In addition to the chugins and their versions (and any errors), `-v5` will show you the default search directories that chuck is scanning for chugins (see below).

Alternately, if you are running chuck programs, you can simply add the `-v` flag (we recommend `-v3` or higher) to whatever chuck files you are running, e.g.,
```
chuck FILE1.ck foo.ck -v3
```
You can also toggle whether to load chugins using the `--chugin-load` command line flag, for example, to run chuck with chugins disabled:
```
chuck  FILE1.ck FILE2.ck --chugin-load:off
```

If you are using **miniAudicle**, you can similarly configure and probe chugins in Preferences. From the main menu, navigate to File->Preferences...->ChuGins tab. Using the `Probe ChuGins` button, for example, will print the chugins diagnostic to the miniAudicle Console Monitor. You can change the verbosity level from the main menu: ChucK->Log Level.


### Chugins Search Paths
By deafult, the chugin loader searches recursively in the following directories for chugins (.chug) and chuck extensions (.ck). It is possible to customize these search paths using the `--chugin-path:<path>` flag with command-line **chuck** or through Preferences/ChuGins tab in **miniAudicle**. Note: some of these requires chuck-1.5.0.4 or higher; we recommend always using the latest released version or the `main` development branch of this repository.
#### macOS
```
/usr/local/lib/chuck
/Library/Application Support/ChucK/ChuGins
~/Library/Application Support/ChucK/ChuGins
~/.chuck/lib
````
#### Linux
```
/usr/local/lib/chuck
~/.chuck/lib/
```
#### Windows
```
C:\WINDOWS\system32\ChucK
C:\Program Files\ChucK\chugins
C:\Program Files (x86)\ChucK\chugins
C:\Users\%USERNAME%\chuck\lib
```

List of Chugins in This Repository
--
- **ABSaturator**
  - soft clip saturating distortion, based on examples from Abel/Berners' Music 424 course at Stanford.
- **Bitcrusher**
  - sample-width reducer + sample rate decimator, bringing to mind the sounds of your favorite low-fidelity vintage audio equipment.
- **MagicSine**
  - Efficient means of computing a sine wave with 2 adds and 2 multiples per sample, at the expense of being able to dynamically adjust phase.
- **KasFilter**
  - by [@Kassen](http://github.com/Kassen)
  - Undersampling-based resonant lowpass filter
- **FIR**
  - by [Perry Cook](http://www.cs.princeton.edu/~prc/)
  - Arbitrary order FIR filter
- **FoldbackSaturator**
  - by [Ness Morris](https://github.com/biikatto)
  - Foldback Saturator for nasty distortion
- **Pan4 / Pan8 / Pan16**
  - Basic equal-power panners for 4, 8, and 16 channels
- **PitchTrack**
  - by [@jwmatthys](https://github.com/jwmatthys)
  - Monophonic autocorrelation pitch tracker, based on [helmholtz~] by Katja, http://www.katjaas.nl/helmholtz/helmholtz.html
- **GVerb**
  - by [@jwmatthys](https://github.com/jwmatthys)
  - Good quality stereo reverb with adjustable parameters
- **Mesh2D**
  - by [@jwmatthys](https://github.com/jwmatthys)
  - STK instrument that simulates a rectilinear, 2-dimensional digital waveguide mesh structure. Basically sounds like striking a metal plate.
- **Spectacle**
  - by [@jwmatthys](https://github.com/jwmatthys)
  - FFT-based spectral delay and EQ
- **Elliptic**
  - by [@jwmatthys](https://github.com/jwmatthys)
  - Elliptic filter, capable of very steep slopes or interesting harmonic ripples
- **MIAP**
  - by [@ericheep](https://github.com/ericheep)
  - Manifold-Interface Amplitude Panner inspired by the research of [Zachary Seldess](http://www.zacharyseldess.com/miap/) and [Steve Ellison](http://www.lightingandsoundamerica.com/reprint/MeyerSpaceMap.pdf).
- **PowerADSR**
  - by [@ericheep](https://github.com/ericheep)
  - Power function ADSR envelope.
- **WarpBuf**
  - by [@dbraun](https://github.com/dbraun)
  - Time-stretching and pitch-stretching sound buffer that can optionally import `asd` files from Ableton Live for beatmatching.
- **WinFuncEnv**
  - by [@ericheep](https://github.com/ericheep)
  - Envelope built on window functions.

Chugins That We Are Working On
--
- Faust (FAUST in ChucK, or FaucK)
- Fluidsynth
- XML, MusicXML
- MAUI for chuck
- ChuGL/GlucK (grahics in in ChucK)
- NHHall
- Convolution Reverb
- Text2Speech synthesizer
- AudioUnit loader
- Ladspa
- Ambisonics
- Rave

Chugins We Would Like to See Happen
--
- More Configurable audio effects: reverbs, dynamics processors, and distortion units
- More synthesis algorithms / physical models
- Algorithmic composition tools (as UGens or new classes)
- Class libraries of all kinds
- A VST loader
- More [Unit Analyzers](https://chuck.stanford.edu/doc/reference/uanae.html) and/or [interactive AI tools](https://chuck.stanford.edu/doc/reference/ai.html)
- Expressive tools for art making
- Tools for education and research
