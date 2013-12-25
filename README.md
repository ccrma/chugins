Joel Matthys' Fork of CCRMA ChuGins
==

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
- **PitchTrack**
  - Monophonic autocorrelation pitch tracker, based on [helmholtz~] by Katja, http://www.katjaas.nl/helmholtz/helmholtz.html
- **GVerb**
  - Good quality stereo reverb

Chugins That I Am Working On
--
- RTcmix's excellent PLACE and MOVE room simulations
- MESH2D STK instrument
- Spectral Delay
