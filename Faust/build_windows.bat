mkdir build
cd build
cmake .. -DFAUST_DIR="~/GitHub/faust"
cmake --build . --config Release
cd ..

cp "libfaust/windows-x86_64/faust.dll" "C:/Program Files (x86)/ChucK/bin/faust.dll"
cp "libfaust/windows-x86_64/sndfile.dll" "C:/Program Files (x86)/ChucK/bin/sndfile.dll"