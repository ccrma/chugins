# WarpBuf

With WarpBuf you can time-stretch and or transpose the pitch of a sound buffer. If don't have an Ableton `.asd` file to go with the audio file, then the BPM will be assumed to be 120. Therefore, to play the file twice as fast, do `240. => myWarpBuf.bpm;`

## Ableton Live Beatmatching

With WarpBuf, you can also use Ableton Live `.asd` files to [warp](https://www.ableton.com/en/manual/audio-clips-tempo-and-warping/) audio files. The warp markers and BPM information in the `.asd` file will affect how ChucK plays the file. The `.asd` should be next to the `.wav` file, so you might have `drums.wav` and `drums.wav.asd`.

Two audio files might have different tempos, but you can "beatmatch" them by giving them the same tempo:

```chuck
120. => warpBuf1.bpm => warpBuf2.bpm;
```

The WarpBuf chugin has a get/set `playhead` property for the current position of the playhead in quarter notes relative to the `.asd`'s `1.1.1` marker. The chugin's `loop_start` and `loop_end` are also relative to `1.1.1`.

## Installation

Update submodules:
`git submodule update --init --recursive`

Create an extra folder for your chugins, `%USERPROFILE%/Documents/ChucK/chugins/`. Create a system environment variable `CHUCK_CHUGIN_PATH` equal to this path.

In `chugins/WarpBuf`, open a command window on Windows or Terminal window on macOS:

```
mkdir build
cd build
cmake ..
```

Then open `chugins/WarpBuf/build/WarpBufChugin.sln` and build in 64-bit Release mode. You should see a new WarpBuf.chug file in the chugins folder you created earlier.

Build 64-bit chuck.exe from [source](https://github.com/ccrma/chuck/tree/main/src/visual-studio). If you want to use [miniAudicle](https://github.com/ccrma/miniAudicle), then miniAudicle must also be 64-bit. Check that your chuck is 64-bit with `chuck --version`.

Run any of the test scripts: `chuck.exe "tests/warpbuf_basic.ck"`. You will need to provide assets in the assets folder.

## Licenses

WarpBuf uses [rubberband](https://github.com/breakfastquay/rubberband/) and [libsamplerate](https://github.com/libsndfile/libsamplerate), so your usage of WarpBuf must obey their licenses.