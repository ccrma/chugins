# WarpBuf

With WarpBuf you can time-stretch and independently transpose the pitch of a sound buffer. If you don't have an Ableton `.asd` file to go with the audio file, then the BPM will be assumed to be 120. Therefore, to play the file twice as fast, do `240. => myWarpBuf.bpm;`

Control parameters:
* .read - ( string , WRITE only ) - loads file for reading
* .playhead - ( float , READ/WRITE ) - set/get playhead position in quarter notes relative to 1.1.1
* .loop - ( int , READ/WRITE ) - toggle looping
* .play - ( int , READ/WRITE ) - toggle playing
* .bpm - ( float , READ/WRITE ) - set/get BPM rate ( Audio files without Ableton warp files are assumed to be 120 BPM )
* .transpose - ( float , READ/WRITE ) - set/get pitch transpose in semitones
* .startMarker ( float , READ/WRITE ) - set/get start marker of the clip
* .endMarker ( float , READ/WRITE ) - set/get end marker of the clip
* .loopStart ( float , READ/WRITE ) - set/get loop start marker of the clip
* .loopEnd ( float , READ/WRITE ) - set/get loop end marker of the clip
* .reset ( float , WRITE ) - reset the internal process buffer of the Rubberband stretcher

## Ableton Live Beatmatching

With WarpBuf, you can also use Ableton Live `.asd` files to [warp](https://www.ableton.com/en/manual/audio-clips-tempo-and-warping/) audio files. The warp markers and implicit BPM information in the `.asd` file will affect how ChucK plays the file. The `.asd` should be next to the `.wav` file, so you might have `drums.wav` and `drums.wav.asd`.

Two audio files might have different tempos, but you can "beatmatch" them by giving them the same tempo:

```chuck
130. => warpBuf1.bpm => warpBuf2.bpm;
```

WarpBuf has been tested with `asd` files created with Ableton Live 9 and 10.1.30.

## Installation

Make sure you have `cmake`, `git`, and `sh` available from the command line/Terminal.

Update submodules:
`git submodule update --recursive`

If on Windows, create an extra folder for your chugins, `%USERPROFILE%/Documents/ChucK/chugins/`. Create a system environment variable `CHUCK_CHUGIN_PATH` equal to this path.

In `chugins/WarpBuf`, open a command window on Windows or Terminal window on macOS and run `sh build_all_platforms.sh`. On Windows, the `WarpBuf.chug` should appear inside `%USERPROFILE%/Documents/ChucK/chugins/`, and on macOS/Linux it should appear in `/usr/local/lib/chuck`.

Run any of the test scripts: `chuck.exe --verbose:10 level:10 "tests/warpbuf_basic.ck"`.

## Licenses

WarpBuf uses [Rubber Band Library](https://github.com/breakfastquay/rubberband/) and [libsamplerate](https://github.com/libsndfile/libsamplerate), so your usage of WarpBuf must obey their licenses.

## Todo:

* Get/set the list of warp markers
* Optionally pre-read the entire audio buffer and hold it in a buffer.
* Support other audio formats (MP3)
