# The quirky thing about this build script is that on macOS, we can't
# use the libsndfile from brew because it's not a static library.
# Therefore, we build libsndfile statically locally and link against it
# in the CMakeLists.txt. On Windows, we can just download libsndfile
# from its GitHub releases.

cd thirdparty/libsndfile
cmake -Bbuild $CMAKEOPTS -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_C_FLAGS="-fPIC" -DCMAKE_CXX_FLAGS="-fPIC" -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX="./install"
cmake --build build --config Release
cmake --build build --target install
cd ../..

cmake -Bbuild . -DCMAKE_VERBOSE_MAKEFILE=ON
cmake --build build --config Release
cmake --build build --config Release --target install