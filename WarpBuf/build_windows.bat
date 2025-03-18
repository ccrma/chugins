rem Download libsndfile
if not exist "thirdparty/libsndfile-1.2.0-win64/" (
    echo "Downloading libsndfile..." 
	cd thirdparty
	curl -OL https://github.com/libsndfile/libsndfile/releases/download/1.2.0/libsndfile-1.2.0-win64.zip
	rem 7z x libsndfile-1.2.0-win64.zip -y
	tar -xf libsndfile-1.2.0-win64.zip
	del /Q libsndfile-1.2.0-win64.zip
	echo "Downloaded libsndfile." 
	cd ..
)

cmake -Bbuild . -DCMAKE_VERBOSE_MAKEFILE=ON -G "Visual Studio 17 2022" -A "x64"
cmake --build build --config Release
rem cmake --build build --config Release --target install