mkdir build
cd build
cmake .. -DFAUST_DIR=$FAUST_DIR -G "Xcode"
cmake --build . --config Release

# todo: if we can avoid this install_name_tool line, then we won't have to codesign manually below.
install_name_tool -change @rpath/libfaust.2.dylib @loader_path/libfaust.2.dylib Release/libFaucK.dylib
codesign --force --deep --sign "$CODESIGN_IDENTITY" Release/libFaucK.dylib

cd ..

mkdir "/usr/local/lib/chuck"

cp "build/Release/libFaucK.dylib" "/usr/local/lib/chuck/Faust.chug"
cp "libfaust/macOS-universal/libfaust.a" "/usr/local/lib/chuck/libfaust.2.dylib"