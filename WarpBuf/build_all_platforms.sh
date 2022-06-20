cmake -Bbuild .
cmake --build build --config Release

if [ $OSTYPE != 'msys'* ] && [ $OSTYPE != 'cygwin'* ]; then
    # we are on macOS
    mkdir "/usr/local/lib/chuck"
    cp "build/libWarpBufChugin.dylib" "/usr/local/lib/chuck/WarpBuf.chug"
fi