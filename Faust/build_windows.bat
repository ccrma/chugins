rem Download libsndfile
if not exist "thirdparty/libsndfile-1.0.31-win64/" (
    echo "Downloading libsndfile..." 
	cd thirdparty
	curl -OL https://github.com/libsndfile/libsndfile/releases/download/1.0.31/libsndfile-1.0.31-win64.zip
	tar -xf libsndfile-1.0.31-win64.zip
	rm libsndfile-1.0.31-win64.zip
	echo "Downloaded libsndfile." 
	cd ..
)

rem Clone Faust
if not exist "thirdparty/faust/" (
    echo "Downloading faust..." 
	cd thirdparty
	git clone https://github.com/grame-cncm/faust.git
	echo "Downloaded faust." 
	cd ..
)

cmake -Bbuild -DFAUST_DIR="thirdparty/faust" -DSndFile_DIR="thirdparty/libsndfile-1.0.31-win64/cmake"
cmake --build build --config Release

cp "libfaust/windows-x86_64/faust.dll" "C:/Program Files (x86)/ChucK/bin/faust.dll"
cp "libfaust/windows-x86_64/sndfile.dll" "C:/Program Files (x86)/ChucK/bin/sndfile.dll"