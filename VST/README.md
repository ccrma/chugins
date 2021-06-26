# VST Chugin

## Patching Chuck

In the root of the chugins directory, there's a git diff file `vst_patch.patch`. Open a command window to the root and apply the patch: `git apply vst_patch.patch`. This will change a file in `chugins/chuck/include`. Don't commit these changes to the repo. Be prepared to undo them once the patch is no longer necessary.

## Installation on Windows

You must clone this repository with command line git in order for `chugins/VST/thirdparty/JUCE_5` to contain an old version of JUCE which contains the VST 2 SDK.

Create an extra folder for your chugins, `%USERPROFILE%/Documents/ChucK/chugins/`. Create a system environment variable `CHUCK_CHUGIN_PATH` equal to this path.

In `chugins/VST`, open a Windows command window:

```
mkdir build
cd build
cmake ..
```

Then open `chugins/VST/build/VST.sln` and build in 64-bit Release mode. You should see a new VST.chug file in the chugins folder you created earlier.

Build 64-bit chuck.exe from [source](https://github.com/ccrma/chuck/tree/main/src/visual-studio). If you want to use [miniAudicle](https://github.com/ccrma/miniAudicle), then miniAudicle must also be 64-bit. Check that your chuck is 64-bit with `chuck --version`.

Modify the test scripts (`chugins/VST/tests/`) so that they use paths to VSTs and presets that exist on your computer. Run them from command line: `chuck.exe vst_instrument_test.ck`.

## License

The VST chugin uses JUCE 5, so you must follow their [license](https://github.com/juce-framework/JUCE/blob/d9dee4d87332bf9e3d12da2e032608698829dc14/LICENSE.md).