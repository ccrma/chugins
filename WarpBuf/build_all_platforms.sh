# The quirky thing about this build script is that on macOS, we can't
# use the libsndfile from brew because it's not a static library.
# Therefore, we build libsndfile statically locally and link against it
# in the CMakeLists.txt. On Windows, we can just download libsndfile
# from its GitHub releases.

if [[ "$OSTYPE" == "linux-gnu"* ]] || [[ "$OSTYPE" == "darwin"* ]]; then
    if [ ! -d thirdparty/libsndfile ]; then
        git clone https://github.com/libsndfile/libsndfile thirdparty/libsndfile
    else
        git -C thirdparty/libsndfile pull
    fi
    cd thirdparty/libsndfile
    LIBSNDFILE_INSTALL_PREFIX="$PWD/install"
    mkdir -p CMakeBuild && cd CMakeBuild
    cmake .. -DCMAKE_INSTALL_PREFIX="$LIBSNDFILE_INSTALL_PREFIX" -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF
    make && make install
    export PKG_CONFIG_PATH="$LIBSNDFILE_INSTALL_PREFIX/lib/pkgconfig"
    cd ../../..
else
    if [ ! -d thirdparty/libsndfile ]; then
        curl -L https://github.com/libsndfile/libsndfile/releases/download/1.2.0/libsndfile-1.2.0-win64.zip -o libsndfile.zip
        7z x libsndfile.zip -y
        mv libsndfile-1.2.0-win64 thirdparty/libsndfile
    fi
fi

cmake -Bbuild .
cmake --build build --config Release

# Copy the chugin to where it can be used by chuck:

if [[ "$OSTYPE" == "darwin"* ]]; then
    # we are on macOS
    mkdir -p "/usr/local/lib/chuck"
    cp "build/libWarpBufChugin.dylib" "/usr/local/lib/chuck/WarpBuf.chug"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # we are on Linux
    mkdir -p "/usr/local/lib/chuck"
    cp "build/libWarpBufChugin.dylib" "/usr/local/lib/chuck/WarpBuf.chug"
elif [ "$(expr substr $(uname -s) 1 10)" = "MINGW32_NT" ] || [ "$(expr substr $(uname -s) 1 10)" = "MINGW64_NT" ] || [ "$(expr substr $(uname -s) 1 9)" = "CYGWIN_NT" ]; then
    echo
else
    echo "Unknown operating system" $(uname -s)
    exit
fi