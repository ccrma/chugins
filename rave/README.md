# TODO
## need to store dlls in same dir as chuck.exe
- is there any way around this?
- ge says to look into static linking
## move code into src/ directory
## get cmake to work with osc/linux


# installatio notes
installing gtest: https://stackoverflow.com/questions/53583286/cmake-file-for-integrated-visual-studio-unit-testing

## build with GPU on windows
follow the instructions here https://stackoverflow.com/questions/56636714/cuda-compile-problems-on-windows-cmake-error-no-cuda-toolset-found

## cmake
make visual studio project:

cmake . -S ..\  -DCMAKE_BUILD_TYPE:STRING=Release -G "Visual Studio 17 2022" -A x64 -DTorch_DIR="C:\Users\nick\Downloads\libtorch-win-shared-with-deps-1.11.0+cpu\libtorch\share\cmake\Torch"

(note the torch_dir is wherever you downloaded libtorch)

build:

cmake --build . --config Release