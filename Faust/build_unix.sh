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
    cmake -Bbuild $CMAKEOPTS -DCMAKE_C_FLAGS="-fPIC" -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_INSTALL_PREFIX="./install"
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

cmake -Bbuild $CMAKEOPTS -DLIBFAUST_DIR="$LIBFAUST_DIR" -DCMAKE_PREFIX_PATH="thirdparty/libsndfile/install"
cmake --build build --config Release
cmake --build build --config Release --target install

# mkdir -p "/usr/local/lib/chuck"
# cp -r "package/fauck-0.0.1/Faust.chug" "/usr/local/lib/chuck/Faust.chug"
