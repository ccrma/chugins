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
    cmake -Bbuild $CMAKEOPTS -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_INSTALL_PREFIX="./install" -DENABLE_EXTERNAL_LIBS=off
    cmake --build build --config Release
    cmake --build build --target install
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
    git checkout a1c3a515abbcafea0a6e4e2ec7ecb0f092de5349
    echo "Downloaded faust."
    cd ../..
fi

cmake -Bbuild $CMAKEOPTS -DLIBFAUST_DIR="$LIBFAUST_DIR" -DSndFile_DIR="thirdparty/libsndfile/install"
cmake --build build --config Release

mkdir -p "/usr/local/lib/chuck"
cp "build/libFaucK.dylib" "/usr/local/lib/chuck/Faust.chug"
cp -r $LIBFAUST_DIR/share/faust /usr/local/share/faust
