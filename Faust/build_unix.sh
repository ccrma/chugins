rm "/usr/local/lib/chuck/Faust.chug"
rm "/usr/local/lib/chuck/libfaust.2.dylib"

if [ "$(uname)" = "Darwin" ]; then
    if [[ $(uname -m) == 'arm64' ]]; then
        export FAUST_LIB_DIR=$PWD/libfaust/darwin-arm64/
    else
        export FAUST_LIB_DIR=$PWD/libfaust/darwin-x86_64/
    fi
else
    export FAUST_LIB_DIR=$PWD/libfaust/ubuntu-x86_64/
fi

mkdir -p $FAUST_LIB_DIR

# if [ "$(uname)" = "Darwin" ]; then
#     echo "You are running macOS"
#     # curl -L https://github.com/DBraun/faust/suites/12075724181/artifacts/635656964 -o faust-2.58.18-arm64.dmg.zip
#     # unzip -o faust-2.58.18-arm64.dmg.zip
#     # hdiutil attach faust-2.58.18-arm64.dmg
#     cp -R /Volumes/Faust-2.58.18/Faust-2.58.18/* $FAUST_LIB_DIR
#     # hdiutil detach /Volumes/Faust-2.58.18/
# elif [ "$(expr substr $(uname -s) 1 5)" = "Linux" ]; then
#     echo "You are running Linux"
# elif [ "$(expr substr $(uname -s) 1 10)" = "MINGW32_NT" ] || [ "$(expr substr $(uname -s) 1 10)" = "MINGW64_NT" ]; then
#     echo "You are running Windows"
#     exit
# else
#     echo "Unknown operating system"
#     exit
# fi

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
    echo "Downloaded faust."
    cd ..
else
    git -C thirdparty/faust pull
fi

cmake -Bbuild $CMAKEOPTS -DFAUST_DIR="thirdparty/faust" -DFAUST_LIB_DIR="$FAUST_LIB_DIR" -DSndFile_DIR="thirdparty/libsndfile/build"
cmake --build build --config Release

mkdir "/usr/local/lib/chuck"
cp "build/libFaucK.dylib" "/usr/local/lib/chuck/Faust.chug"
