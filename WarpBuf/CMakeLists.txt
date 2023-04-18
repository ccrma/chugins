cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

project(WarpBufChugin VERSION 0.0.1)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT WarpBufChugin)

project(WarpBufChugin VERSION 0.0.1)

## libsamplerate - start
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate)
## libsamplerate - end

## Rubberband - start
# List of modules to build
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
## Rubberband - end

#######################################
# libsndfile
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

    # need lame due to brew's lame not having a .pc file
    set(LAME_LIB_DIR ${LAME_DIR}/lib)

endif()

FILE(GLOB CK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include/*.h
)

set(Sources
    "src/AbletonClipInfo.h"
    "src/WarpBufChugin.h"
    "src/WarpBufChugin.cpp"
    "src/WarpBufChuginDLL.cpp"
)

source_group("Sources" FILES ${Sources})

add_library(WarpBufChugin SHARED ${CK_SOURCES} ${Sources} ${RUBBERBAND_SOURCES} ${RUBBERBAND_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/include/samplerate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/portable_endian/include/portable_endian.h
    )
set_target_properties(WarpBufChugin PROPERTIES CXX_STANDARD 17)

target_compile_definitions(WarpBufChugin
    PUBLIC
    HAVE_LIBSAMPLERATE
    _USRDLL
    NDEBUG
    _NDEBUG
    _MBCS
    WarpBufChugin_EXPORTS
    _LIB
)

if(MSVC)
    target_compile_definitions(WarpBufChugin
        PUBLIC
        USE_BUILTIN_FFT
        NOMINMAX
        _WINDOWS
        WIN32
        __WINDOWS_MODERN__
        __PLATFORM_WIN32__
        __WINDOWS_DS__
        )
elseif(APPLE)
    target_compile_definitions(WarpBufChugin
        PUBLIC
        HAVE_VDSP
        __MACOSX_CORE__
        USE_PTHREADS
    )   
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC")
else()
    target_compile_definitions(WarpBufChugin
        PUBLIC
        USE_BUILTIN_FFT
        __PLATFORM_LINUX__
        USE_PTHREADS
    )   
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC")
endif()

target_link_libraries(WarpBufChugin samplerate)
if(APPLE)
target_link_libraries(WarpBufChugin "-framework Accelerate")
endif()

target_include_directories(WarpBufChugin PRIVATE ${SNDFILE_INCLUDE_DIRS})
if (DEFINED SNDFILE_IS_STATIC)
target_link_directories(WarpBufChugin PRIVATE ${LAME_LIB_DIR})
target_link_libraries (WarpBufChugin PkgConfig::SNDFILE PkgConfig::FLAC PkgConfig::VORBIS PkgConfig::OGG PkgConfig::OPUS PkgConfig::MPG123)
else()
target_link_libraries (WarpBufChugin ${SNDFILE_LIBRARIES})
endif()

# Include header directories
target_include_directories(WarpBufChugin PUBLIC
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

if(MSVC)
    target_compile_options(WarpBufChugin PRIVATE
        $<$<CONFIG:Debug>:
            /Od;
            /RTC1;
            /MDd;
        >
        $<$<CONFIG:Release>:
            /MD;
        >
        /W3;
        /Zi;
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-;
    )
    target_link_options(WarpBufChugin PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /DEBUG;
        /SUBSYSTEM:WINDOWS;
        /INCREMENTAL:NO
    )
endif()

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