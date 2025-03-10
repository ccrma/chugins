cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR})

if (DEFINED ENV{WARPBUF_VERSION})
set (VERSION $ENV{WARPBUF_VERSION})
else()
set (VERSION "0.0.1")
endif()

project(WarpBuf VERSION ${VERSION})

# Set startup project for Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT WarpBuf)

if (UNIX AND NOT APPLE)
   SET(CMAKE_SKIP_BUILD_RPATH FALSE)
   set(CMAKE_INSTALL_RPATH "$ORIGIN")
   set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

# Collect ChucK sources
FILE(GLOB CK_SOURCES ${PROJECT_SOURCE_DIR}/../../chuck/include/*.h)
source_group("ChucK" FILES ${CK_SOURCES})

# Collect WarpBuf sources
set(Sources
    "src/AbletonClipInfo.h"
    "src/WarpBufChugin.h"
    "src/WarpBufChugin.cpp"
    "src/WarpBufChuginDLL.cpp"
)
source_group("Sources" FILES ${Sources})

# Add Rubber Band sources
set(RUBBERBAND_SRC_CPP
    src/RubberBandStretcher.cpp
    src/common/StretchCalculator.cpp
    src/common/Profiler.cpp
    src/common/Resampler.cpp
    src/common/FFT.cpp
    src/common/Log.cpp
    src/common/mathmisc.cpp
    src/common/sysutils.cpp
    src/common/Thread.cpp
    src/faster/AudioCurveCalculator.cpp
    src/faster/CompoundAudioCurve.cpp
    src/faster/HighFrequencyAudioCurve.cpp
    src/faster/R2Stretcher.cpp
    src/faster/SilentAudioCurve.cpp
    src/faster/PercussiveAudioCurve.cpp
    src/faster/StretcherChannelData.cpp
    src/faster/StretcherProcess.cpp
    src/finer/R3Stretcher.cpp
)
# Convert to list of source files (extension based on OS)
foreach(cpp_file IN LISTS RUBBERBAND_SRC_CPP)
    list(APPEND RUBBERBAND_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rubberband/${cpp_file})
endforeach()

set(RUBBERBAND_HEADERS 
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rubberband/rubberband/RubberBandStretcher.h
)

# Create shared library
add_library(WarpBuf MODULE ${CK_SOURCES} ${Sources} ${RUBBERBAND_SOURCES} ${RUBBERBAND_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/include/samplerate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/portable_endian/include/portable_endian.h
)

####################################
# Install - used for packaging
####################################

set (PACKDIR ${ROOT}/package)
set (DST warpbuf-${VERSION})
install ( TARGETS WarpBuf RUNTIME_DEPENDENCIES
    RUNTIME DESTINATION ${PACKDIR}/${DST}
    LIBRARY DESTINATION ${PACKDIR}/${DST}
    ARCHIVE DESTINATION ${PACKDIR}/${DST}
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_compile_options(-fPIC)

if(APPLE)
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINK_FLAGS "-Wl,-U,_object_new_imp -Wl,-U,_object_method_imp"
    BUNDLE TRUE
    BUNDLE_EXTENSION chug
    OUTPUT_NAME WarpBuf
    MACOSX_BUNDLE_GUI_IDENTIFIER edu.stanford.chuck.${PROJECT_NAME}
    MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
    MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION}
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist
)
else()
set_target_properties (${PROJECT_NAME} PROPERTIES 
    SUFFIX ".chug"
    PREFIX ""
    OUTPUT_NAME WarpBuf)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_LIBSAMPLERATE)

# Platform-specific libraries and definitions
if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        HAVE_VDSP
        USE_PTHREADS
        __MACOSX_CORE__
        __APPLE__
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreFoundation" "-framework CoreMIDI" "-framework CoreAudio" "-framework Accelerate")
elseif(MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 winmm wsock32 dsound dinput8 dxguid)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        USE_BUILTIN_FFT
        WIN32
        _WIN32
        _WINDOWS
        NOMINMAX
        __WINDOWS_DS__
        __PLATFORM_WIN32__
    )
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _GLIBCXX_USE_CXX11_ABI=0
        USE_BUILTIN_FFT
        USE_PTHREADS
        __PLATFORM_LINUX__
    )
endif()

# Find sndfile and link it
if (MSVC)
    set(SndFile_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsndfile-1.2.0-win64/cmake")
    find_package(SndFile REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE SndFile::sndfile)
else()
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsndfile/install")
    find_package(PkgConfig REQUIRED)
    # We expect the user to have used brew to install the dependencies
    # for libsndfile, to have built libsndfile as a static lib,
    # and for it to be accessible via `pkg-config --static libsndfile`.
    if(APPLE)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    list(APPEND PKG_CONFIG_EXECUTABLE "--static")  # append temporarily
    endif()
    pkg_check_modules(SNDFILE REQUIRED IMPORTED_TARGET sndfile)
    pkg_check_modules(FLAC REQUIRED IMPORTED_TARGET flac)
    pkg_check_modules(VORBIS REQUIRED IMPORTED_TARGET vorbis)
    pkg_check_modules(OGG REQUIRED IMPORTED_TARGET ogg)
    pkg_check_modules(OPUS REQUIRED IMPORTED_TARGET opus)
    pkg_check_modules(MPG123 REQUIRED IMPORTED_TARGET libmpg123)
    if(APPLE)
    execute_process(COMMAND find /usr/local/Cellar/lame -name libmp3lame.a OUTPUT_VARIABLE SNDFILE_STATIC_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
    list(POP_BACK PKG_CONFIG_EXECUTABLE)  # undo the append above
    endif()
    target_link_libraries (${PROJECT_NAME} PRIVATE PkgConfig::SNDFILE PkgConfig::FLAC PkgConfig::VORBIS PkgConfig::OGG PkgConfig::OPUS PkgConfig::MPG123 ${SNDFILE_STATIC_LIBS})
endif()

# Add libsamplerate project
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate)
target_link_libraries(${PROJECT_NAME} PRIVATE samplerate)

# Include header directories
target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rubberband>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rubberband/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/portable_endian/include>
    $<INSTALL_INTERFACE:WarpBuf>
)

if (MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES
                        VS_DEBUGGER_COMMAND "C:\\Program\ Files\ (x86)\\ChucK\\bin\\chuck.exe"
                        VS_DEBUGGER_COMMAND_ARGUMENTS "..\\tests\\warpbuf_basic.ck")

  add_custom_command(TARGET ${PROJECT_NAME}
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     "$<TARGET_FILE:WarpBuf>"
                     "%USERPROFILE%/Documents/ChucK/chugins/WarpBuf.chug")
endif()

if (WIN32)
####################################
# install sndfile
install (
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsndfile-1.2.0-win64/bin/sndfile.dll DESTINATION ${PACKDIR}/${DST}
)
endif()

if (APPLE)
install (
    CODE "execute_process (COMMAND codesign --sign - --deep --force ${PACKDIR}/${DST}/WarpBuf.chug)" 
    )
endif()
