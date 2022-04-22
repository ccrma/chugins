# DbDexed Chugin

## Intro

DbDexed is a port of [Dexed](https://github.com/asb2m10/dexed) (GPL3 licensed)
to the ChucK/chugin environment.  Dexed, in turn, is a port of 
[music synth for android](https://github.com/google/music-synthesizer-for-android) 
(Apache license) to the VST3 environment.  Both flavors of Dexed can
be made to work with ChucK, however this chugin is slightly more lean
and mean since the VST3 interfaces don't get in the way.

Dexed is designed to replicate the DX7 audio processing pipeline which
is a 6-unit pipelined FM system.  We shim the 24-bit fixed-point audio 
synth into ChucK's floating point tick.  The DbMdaDX10 chugin is 
similar but much less rigorous in its DX7 emulation.

Here, the goal is to ensure that existing DX7 presets/patches produce 
a useable result in the ChucK environment.

Because Dexed has a multitude of parameters (more than 150) we currently
support parameter-specification via cartridges and presets.  We've included
the same set of cartridges that Dexed includes in the `presets` subdirectory.

```sh
# The entire set of cartridges can easily be auditioned 
% cd test
% chuck cartridges.ck
```

More presets/cartridges can likely be found on the internet.  You can
also develop your own cartridge with the Dexed GUI and then use those
with DbDexed.

## Example

```ck
// The Dexed default cartridge is built-in to DbDexed.
// This means you have 32 voices at your disposal without loading a cartridge.
DbDexed inst => dac;
.1::second => dur noteDur;
[60, 62, 64, 65, 67, 69, 71] @=> int notes[];
for(int i;i<notes.size();i++)
{
    notes[i] => int n;
    inst.noteOn(n, 1);
    noteDur => now;
    inst.noteOff(n, 1);
    noteDur => now;
}
2::second => now; // ring
```

## API

### Making Sound

| Method                                  | Description                 |
| :-------------------------------------- | :-------------------------- |
| `midiEvent(MidiMsg msg)`                | Performs the MIDI event.    |
| `noteOn(int midiNote, float velocity)`  | Simpler way to play a note. |
| `noteOff(int midiNote, float velocity)` |                             |

### Presets and Voices

| Method                               | Description                                                                |
| :----------------------------------- | :------------------------------------------------------------------------- |
| `int loadCartridge(string filepath)` | Loads the requested Dexed cartridge file. Returns non-zero value on error. |
| `printPresets()`                     | Prints the list of 32 presets in the current cartridge.                    |
| `string getPresetName(int i)`        | Returns the name of the ith preset in the current cartridge.               |
| `selectPreset(int i)`                | Activates the ith preset in the current cartridge.                         |
| `int getNumPresets()`                | Returns 32, since all carts have the same number of presets.               |



