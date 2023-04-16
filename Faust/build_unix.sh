rm "/usr/local/lib/chuck/Faust.chug"
rm "/usr/local/lib/chuck/libfaust.2.dylib"

mkdir thirdparty

# Clone libsndfile
if [ ! -d "thirdparty/libsndfile/" ] 
then
    echo "Downloading libsndfile..." 
    cd thirdparty
    git clone https://github.com/libsndfile/libsndfile.git
    echo "Downloaded libsndfile." 
    echo "Building libsndfile"
    cd libsndfile
    cmake -Bbuild -DCMAKE_BUILD_TYPE=Release $CMAKEOPTS -DCMAKE_VERBOSE_MAKEFILE=ON -DENABLE_EXTERNAL_LIBS=off
    cmake --build build --config Release
    cd ../..
fi

# Clone Faust
if [ ! -d "thirdparty/faust/" ] 
then
    echo "Downloading faust..." 
    cd thirdparty
    git clone https://github.com/grame-cncm/faust.git
    echo "Downloaded faust."
    cd ..
fi

cmake -Bbuild -DCMAKE_BUILD_TYPE=Release $CMAKEOPTS -G "Xcode" -DFAUST_DIR="thirdparty/faust" -DSndFile_DIR="thirdparty/libsndfile/build"
cmake --build build --config Release

mkdir "/usr/local/lib/chuck"

if [[ $OSTYPE == 'darwin'* ]]; then
    # we are on macOS
    cp "libfaust/macOS-universal/libfaust.a" "/usr/local/lib/chuck/libfaust.2.dylib"
else
    # we are on Linux
    if [[ $(uname -m) == 'x86_64' ]]; then
        echo "building for x86_64"
        cp "libfaust/ubuntu-x86_64/libfaust.so.2" "/usr/local/lib/chuck/libfaust.so.2"
    else
        cp "libfaust/ubuntu-$(uname -m)/libfaust.so.2" "/usr/local/lib/chuck/libfaust.so.2"
    fi

    ln "/usr/local/lib/chuck/libfaust.so.2" "/usr/local/lib/chuck/libfaust.so"
fi

cp "build/Release/libFaucK.dylib" "/usr/local/lib/chuck/Faust.chug"
