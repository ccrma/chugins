rm "/usr/local/lib/chuck/Faust.chug"
rm "/usr/local/lib/chuck/libfaust.2.dylib"

if [ "$(uname)" = "Darwin" ]; then
    if [[ $(uname -m) == 'arm64' ]]; then
        export LIBFAUST_DIR=$PWD/thirdparty/libfaust/darwin-arm64/Release
    else
        export LIBFAUST_DIR=$PWD/thirdparty/libfaust/darwin-x64/Release
    fi
else
    export LIBFAUST_DIR=$PWD/thirdparty/libfaust/ubuntu-x86_64/Release
fi

# Clone libsndfile
if [ ! -d "thirdparty/libsndfile/" ] 
then
    echo "Downloading libsndfile..." 
    cd thirdparty
    git clone https://github.com/libsndfile/libsndfile.git
    echo "Downloaded libsndfile." 
    echo "Building libsndfile"
    cd libsndfile
    cmake -Bbuild $CMAKEOPTS -DCMAKE_VERBOSE_MAKEFILE=ON -DENABLE_EXTERNAL_LIBS=off
    cmake --build build --config Release
    cd ../..
else
    git -C thirdparty/libsndfile pull
fi

# Clone Faust
if [ ! -d "thirdparty/faust/" ] 
then
    echo "Downloading faust..." 
    cd thirdparty
    git clone https://github.com/grame-cncm/faust.git
    cd faust
    git checkout 28a5eacb0acbb80203b93ee71663d9a097536641
    echo "Downloaded faust."
    cd ../..
else
    git -C thirdparty/faust pull
fi

cmake -Bbuild $CMAKEOPTS -DFAUST_DIR="thirdparty/faust" -DLIBFAUST_DIR="$LIBFAUST_DIR" -DSndFile_DIR="thirdparty/libsndfile/build"
cmake --build build --config Release

mkdir "/usr/local/lib/chuck"
cp "build/libFaucK.dylib" "/usr/local/lib/chuck/Faust.chug"
