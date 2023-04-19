cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

project(WarpBufChugin VERSION 0.0.1)

# Set startup project for Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT WarpBufChugin)

# Collect ChucK sources
FILE(GLOB CK_SOURCES ${PROJECT_SOURCE_DIR}/../../chuck/include/*.h)
source_group("ChucK" FILES ${CK_SOURCES})

# Collect WarpBufChugin sources
set(Sources
    "src/AbletonClipInfo.h"
    "src/WarpBufChugin.h"
    "src/WarpBufChugin.cpp"
    "src/WarpBufChuginDLL.cpp"
)
source_group("Sources" FILES ${Sources})

# Add libsamplerate project
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate)

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
add_library(WarpBufChugin SHARED ${CK_SOURCES} ${Sources} ${RUBBERBAND_SOURCES} ${RUBBERBAND_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/include/samplerate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/portable_endian/include/portable_endian.h
)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_LIBSAMPLERATE)

# Platform-specific libraries and definitions
if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "HAVE_VDSP;USE_PTHREADS;__MACOSX_CORE__;__APPLE__")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreFoundation" "-framework CoreMIDI" "-framework CoreAudio" "-framework Accelerate")
elseif(MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 winmm wsock32 dsound dinput8 dxguid)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "USE_BUILTIN_FFT;WIN32;_WIN32;_WINDOWS;NOMINMAX;__WINDOWS_DS__;__WINDOWS_MODERN__;__PLATFORM_WIN32__")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE "USE_BUILTIN_FFT;USE_PTHREADS;__PLATFORM_LINUX__")
endif()

target_link_libraries(WarpBufChugin PRIVATE samplerate)

# link libsndfile statically
if (MSVC)
    set (SNDFILE_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsndfile/lib/sndfile.lib)
    set (SNDFILE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsndfile/include)
else()
    # We expect the user to have built libsndfile as a static lib
    # and for it to be accessible via `pkg-config --static libsndfile`
    set(SNDFILE_IS_STATIC 1)
    find_package(PkgConfig REQUIRED)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    list(APPEND PKG_CONFIG_EXECUTABLE "--static")  # append temporarily
    pkg_check_modules(SNDFILE REQUIRED IMPORTED_TARGET sndfile)
    pkg_check_modules(FLAC REQUIRED IMPORTED_TARGET flac)
    pkg_check_modules(VORBIS REQUIRED IMPORTED_TARGET vorbis)
    pkg_check_modules(OGG REQUIRED IMPORTED_TARGET ogg)
    pkg_check_modules(OPUS REQUIRED IMPORTED_TARGET opus)
    pkg_check_modules(MPG123 REQUIRED IMPORTED_TARGET libmpg123)
    list(POP_BACK PKG_CONFIG_EXECUTABLE)  # undo the append above

endif()
target_include_directories(WarpBufChugin PRIVATE ${SNDFILE_INCLUDE_DIRS})
if (DEFINED SNDFILE_IS_STATIC)
target_link_libraries (WarpBufChugin PRIVATE PkgConfig::SNDFILE PkgConfig::FLAC PkgConfig::VORBIS PkgConfig::OGG PkgConfig::OPUS PkgConfig::MPG123)
else()
target_link_libraries (WarpBufChugin PRIVATE ${SNDFILE_LIBRARIES})
endif()

# Include header directories
target_include_directories(WarpBufChugin PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Rubberband>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Rubberband/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/portable_endian/include>
    $<INSTALL_INTERFACE:WarpBufChugin>
)

# Install library
INSTALL(TARGETS WarpBufChugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES
                        VS_DEBUGGER_COMMAND "C:\\Program\ Files\ (x86)\\ChucK\\bin\\chuck.exe"
                        VS_DEBUGGER_COMMAND_ARGUMENTS "..\\tests\\warpbuf_basic.ck")

  add_custom_command(TARGET ${PROJECT_NAME}
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     "$<TARGET_FILE:WarpBufChugin>"
                     "%USERPROFILE%/Documents/ChucK/chugins/WarpBuf.chug")
endif()