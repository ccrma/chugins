Repository for Officially Supported ChuGins
==

Prepackaged Binaries
--
[CCRMA ChuGins for Mac OS X](https://github.com/downloads/ccrma/chugins/chugins-mac-2012-10-16.tgz)

[CCRMA ChuGins for Windows](https://github.com/downloads/ccrma/chugins/chugins-windows-2012-10-16.zip)

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

Chugins That We Are Working On
--
- MAUI for chuck
- GlucK (OpenGL)
- AudioUnit loader

Chugins We would like to see happen
--
- More configurable reverbs, dynamics processors, and distortion units
- VST loader
- Ambisonics
- General spatial processing
