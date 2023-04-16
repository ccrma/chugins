rem mkdir libfaust\windows-x86_64
rem Faust-2.58.18-win64.exe /S /D=%cd%\libfaust\windows-x86_64

rem set CHUCK_DIR="C:/Program Files (x86)/ChucK/"
set CHUCK_DIR=C:/Program Files/chuck/

mkdir thirdparty

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
	echo "Downloaded faust." 
	cd ..
)

cmake -Bbuild -DFAUST_DIR="thirdparty/faust" -DSndFile_DIR="thirdparty/libsndfile-1.2.0-win64/cmake"
cmake --build build --config Release

cp "libfaust/windows-x86_64/lib/faust.dll" "%CHUCK_DIR%/bin/faust.dll"
xcopy "libfaust/windows-x86_64/share/faust" "%CHUCK_DIR%/share/faust" /E /I /D
cp "thirdparty/libsndfile-1.2.0-win64/bin/sndfile.dll" "%CHUCK_DIR%/bin/sndfile.dll"