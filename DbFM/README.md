# DbFM Chugin

DbFM is a port of the [music synth for android](github.com/google/music-synthesizer-for-android) 
project (Apache license) to the ChucK/chugin environment.  The DbMdaDX10
chugin is similar but much less rigorous in its DX10 emulation.

The synth is designed to replicate the DX7 audio processing pipeline which
is a 6-unit pipelined FM system.  We shim the 16-bit audio synth into
ChucK's floating point tick.

The goal is to ensure that existing DX7 presets/patches produce a useable 
result in this environment.

The [Dexed Project](github.com/asb2m10/dexed) is an open-source (GPL) project
that leverages the same synth engine. It targets a VST3 plugin environment and
sports a fancy-pants UI that may make patch-development easier.


