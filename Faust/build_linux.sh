mkdir build
cd build
cmake .. -DFAUST_DIR=$FAUST_DIR
cmake --build . --config Release
cd ..

mkdir "/usr/local/lib/chuck"

cp "build/Release/libFaucK.dylib" "/usr/local/lib/chuck/Faust.chug"

if [[ $(uname -m) == 'x86_64' ]]; then
    echo "building for x86_64"
	cp "libfaust/ubuntu-x86_64/libfaust.so.2" "/usr/local/lib/chuck/libfaust.so.2"
else
	cp "libfaust/ubuntu-$(uname -m)/libfaust.so.2" "/usr/local/lib/chuck/libfaust.so.2"
fi

ln "/usr/local/lib/chuck/libfaust.so.2" "/usr/local/lib/chuck/libfaust.so"