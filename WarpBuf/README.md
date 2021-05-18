# WarpBuf

With WarpBuf you can time-stretch and or transpose the pitch of a sound buffer. If don't have an Ableton `.asd` file to go with the audio file, then the BPM will be assumed to be 120. Therefore, to play the file twice as fast, do `240. => myWarpBuf.bpm;`

# Ableton Live Beatmatching

With WarpBuf, you can also use Ableton Live `.asd` files to [warp](https://www.ableton.com/en/manual/audio-clips-tempo-and-warping/) audio files. The warp markers and BPM information in the `.asd` file will affect how ChucK plays the file. The `.asd` should be next to the `.wav` file, so you might have `drums.wav` and `drums.wav.asd`.

Two audio files might have different tempos, but you can "beatmatch" them by giving them the same tempo:

```chuck
120. => warpBuf1.bpm => warpBuf2.bpm;
```

The WarpBuf chugin has a get/set `playhead` property for the current position of the playhead in quarter notes relative to the `.asd`'s `1.1.1` marker. The chugin's `loop_start` and `loop_end` are also relative to `1.1.1`.