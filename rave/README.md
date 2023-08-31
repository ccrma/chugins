# Installation Instructions
## Linux
Copy the contents of the `rave_chugin_release` folder to `/usr/local/lib/chuck`.

If you are building on your machine, call `sudo cmake --build . --config Release --target install` to install everything.

# TODO
## need to store dlls in same dir as chuck.exe
- is there any way around this?
- ge says to look into static linking
## move code into src/ directory
## get cmake to work with osc/linux


# Installation Instructions
installing gtest (not necessary, don't worry about this): https://stackoverflow.com/questions/53583286/cmake-file-for-integrated-visual-studio-unit-testing

## Windows
## build with GPU on windows
follow the instructions here https://stackoverflow.com/questions/56636714/cuda-compile-problems-on-windows-cmake-error-no-cuda-toolset-found


### cmake
How to make visual studio project:

- First download [libtorch](https://pytorch.org/get-started/locally/). In the selector, pick `Stable > Windows > LibTorch > C++/Java > CPU`. Then download the release version. Note: If building for the gpu, you will probably need to install the same CUDA version as your libtorch download (you can install mulitple versions at the same time).
- Unzip the file.
- Make sure CMake is installed
- Go into the terminal (preferably powershell or cygwin).
- Navigate to the `chugins/rave/` directory.
- `git submodule update --init`
- Make a `build` directory
- `cd build`
- `cmake . -S ..\  -DCMAKE_BUILD_TYPE:STRING=Release -G "Visual Studio 17 2022" -A x64 -DTorch_DIR="<Path to LibTorch>\libtorch\share\cmake\Torch"`

How to build:
- Navigate to build directory from above
- `cmake --build . --config Release`
- `sudo cmake --build . --config Release --target install` to install
