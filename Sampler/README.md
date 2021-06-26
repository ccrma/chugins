# Sampler Chugin

## Patching Chuck

In the root of the chugins directory, there's a git diff file `vst_patch.patch`. Open a command window to the root and apply the patch: `git apply vst_patch.patch`. This will change a file in `chugins/chuck/include`. Don't commit these changes to the repo. Be prepared to undo them once the patch is no longer necessary.

## Installation on Windows

Update submodules:
`git submodule update --init --recursive`

Create an extra folder for your chugins, `%USERPROFILE%/Documents/ChucK/chugins/`. Create a system environment variable `CHUCK_CHUGIN_PATH` equal to this path.

In `chugins/Sampler`, open a Windows command window:

```
mkdir build
cd build
cmake ..
```

Then open `chugins/Sampler/build/Sampler.sln` and build in 64-bit Release mode. You should see a new Sampler.chug file in the chugins folder you created earlier.

Build 64-bit chuck.exe from [source](https://github.com/ccrma/chuck/tree/main/src/visual-studio). If you want to use [miniAudicle](https://github.com/ccrma/miniAudicle), then miniAudicle must also be 64-bit. Check that your chuck is 64-bit with `chuck --version`.

Run any of the test scripts: `chuck.exe "tests/sampler_test.ck"`.

## License

The Sampler chugin uses [Sampler](https://github.com/DBraun/Sampler), which uses JUCE, so you must follow all of the licenses.