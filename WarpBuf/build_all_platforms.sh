if [ "$(expr substr $(uname -s) 1 10)" = "MINGW32_NT" ] || [ "$(expr substr $(uname -s) 1 10)" = "MINGW64_NT" ] || [ "$(expr substr $(uname -s) 1 9)" = "CYGWIN_NT" ]; then
    # download libsndfile
    if [ ! -d thirdparty/libsndfile ]; then
        curl -L https://github.com/libsndfile/libsndfile/releases/download/1.2.0/libsndfile-1.2.0-win64.zip -o libsndfile.zip
        7z x libsndfile.zip -y
        mv libsndfile-1.2.0-win64 thirdparty/libsndfile
    fi
fi

cmake -Bbuild .
cmake --build build --config Release

if [ "$(uname)" = "Darwin" ]; then
    # we are on macOS
    echo "we are on macOS"
    mkdir "/usr/local/lib/chuck"
    cp "build/libWarpBufChugin.dylib" "/usr/local/lib/chuck/WarpBuf.chug"
elif [ "$(expr substr $(uname -s) 1 5)" = "Linux" ]; then
    # we are on Linux
    echo "we are on linux"
    mkdir "/usr/local/lib/chuck"
    cp "build/libWarpBufChugin.dylib" "/usr/local/lib/chuck/WarpBuf.chug"
elif [ "$(expr substr $(uname -s) 1 10)" = "MINGW32_NT" ] || [ "$(expr substr $(uname -s) 1 10)" = "MINGW64_NT" ] || [ "$(expr substr $(uname -s) 1 9)" = "CYGWIN_NT" ]; then
    echo
else
    echo "Unknown operating system" $(uname -s)
    exit
fi