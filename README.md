Repository for Officially Supported ChuGins [![Build Status](https://travis-ci.org/ccrma/chugins.svg?branch=master)](https://travis-ci.org/ccrma/chugins)
==

Prepackaged Binaries
--
Binaries for most chugins in this repository are automatically included by the ChucK installers for Mac OS X and Windows. If you haven't installed ChucK already, these are available at [http://chuck.stanford.edu/release/](http://chuck.stanford.edu/release/). 

### Linux/Advanced Users
To compile and install all chugins:

    git clone https://github.com/ccrma/chugins.git
    cd chugins
    make [osx|win32|linux]
    sudo make install

List of Current Chugins
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
- **PowerADSR**
  - by [@ericheep](https://github.com/ericheep)
  - Power function ADSR envelope.
- **WinFuncEnv**
  - by [@ericheep](https://github.com/ericheep)
  - Envelope built on window functions.

Chugins That We Are Working On
--
- MAUI for chuck
- GlucK (OpenGL)
- AudioUnit loader
- Ambisonics

Chugins We would like to see happen
--
- More configurable reverbs, dynamics processors, and distortion units
- VST loader
