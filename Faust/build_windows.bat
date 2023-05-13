set CHUCK_DIR=C:/Program Files/ChucK

rem Download libsndfile
if not exist "thirdparty/libsndfile-1.2.0-win64/" (
    echo "Downloading libsndfile..." 
	cd thirdparty
	curl -OL https://github.com/libsndfile/libsndfile/releases/download/1.2.0/libsndfile-1.2.0-win64.zip
	7z x libsndfile-1.2.0-win64.zip -y
	rm libsndfile-1.2.0-win64.zip
	echo "Downloaded libsndfile." 
	cd ..
)

rem Clone Faust
if not exist "thirdparty/faust/" (
    echo "Downloading faust..." 
	cd thirdparty
	git clone https://github.com/grame-cncm/faust.git
	cd faust
	git checkout 28a5eacb0acbb80203b93ee71663d9a097536641
	echo "Downloaded faust." 
	cd ../..
)

cmake -Bbuild -DFAUST_DIR="thirdparty/faust" -DLIBFAUST_DIR="thirdparty/libfaust/win64/Release" -DSndFile_DIR="thirdparty/libsndfile-1.2.0-win64/cmake"
cmake --build build --config Release

xcopy "thirdparty/libfaust/win64/Release/share/faust" "%CHUCK_DIR%/../share/faust" /E /I /D
cp "thirdparty/libsndfile-1.2.0-win64/bin/sndfile.dll" "%CHUCK_DIR%/sndfile.dll"