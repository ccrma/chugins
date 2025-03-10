# FluidSynth

A chugin for loading [FluidSynth soundfonts](https://en.wikipedia.org/wiki/FluidSynth).

## Building the chump package

If you're building `FluidSynth` for your own use, you can just call `make mac/win/linux/etc`. These instructions are specific to building `FluidSynth.chug` to be packages with chump.

### Mac

Note: Currenty the `meson.build` does not work as a cross-platform
build, so you will need to build the arm64 and x86_64 versions of the
`FluidSynth.chug` on their respective machines before making a
universal binary.

The mac build process is a little involved due to the need to make a
universal binary, codesigning, and notarization. It requires and
apple developer account to do all this.

First, compile FluidSynth. This requires meson:

```make build-mac-x86_64 # must be done on an intel machine```
```make build-mac-arm64  # must be done on an M-series mac```

Next, we being the codesigning process.

```sh chump_build_mac/1-codesign.sh```

The next step is to take the chugin, and related files and created a new package
for chump (be sure to change the version number!
```sh chump_build_mac/2-chumpinate.sh```

Finally, we notarize the outputted `FluidSynth_mac.zip`:
``` sh chump_build_mac/3-notarize.sh```

Afterwards, we upload this to ccrma servers:
```
ssh nshaheed@ccrma-gate.stanford.edu \"mkdir -p ~/Library/Web/chugins/FluidSynth/<verno>/mac/
scp FluidSynth_mac.zip nshaheed@ccrma-gate.stanford.edu:~/Library/Web/chugins/FluidSynth/<verno>/mac
```

And then we add this new version definition to the chump manifest repo:
```
cp -r FluidSynth/ <path_to_chump_packages_repo>
```

### Windows

Note: this requires `meson` to build `FluidSynth` as a chump package.

First, setup your build directory:

```meson setup --buildtype=release builddir-release --backend vs```

Next, we build the chugin:

```meson compile -C builddir-release```

After this, we create a chump package:

```chuck build-pkg-win.ck```

Afterwards, we upload this to ccrma servers:
```
ssh nshaheed@ccrma-gate.stanford.edu \"mkdir -p ~/Library/Web/chugins/FluidSynth/<verno>/win/
scp FluidSynth_win.zip nshaheed@ccrma-gate.stanford.edu:~/Library/Web/chugins/FluidSynth/<verno>/win
```

And then we add this new version definition to the chump manifest repo:
```
cp -r FluidSynth/ <path_to_chump_packages_repo>
```


### Linux
