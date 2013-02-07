Repository for Officially Supported ChuGins
==

Prepackaged Binaries
--
[CCRMA ChuGins for Mac OS X](http://chuck.stanford.edu/chugins/chugins-mac-2013-02-06.tgz)

[CCRMA ChuGins for Windows](http://chuck.stanford.edu/chugins/chugins-windows-2013-02-06.zip)

Installation instructions are included in the binary packages. 

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
  - Undersampling-based resonant lowpass filter by Kassen
- **FIR**
  - Arbitrary order FIR filter by Perry Cook
- **Pan4 / Pan8 / Pan16**
  - Basic equal-power panners for 4, 8, and 16 channels

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
