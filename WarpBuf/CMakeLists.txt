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
    src/rubberband-c.cpp
    src/RubberBandStretcher.cpp
    src/StretcherProcess.cpp
    src/StretchCalculator.cpp
    src/base/Profiler.cpp
    src/dsp/AudioCurveCalculator.cpp
    src/audiocurves/CompoundAudioCurve.cpp
    src/audiocurves/SpectralDifferenceAudioCurve.cpp
    src/audiocurves/HighFrequencyAudioCurve.cpp
    src/audiocurves/SilentAudioCurve.cpp
    src/audiocurves/ConstantAudioCurve.cpp
    src/audiocurves/PercussiveAudioCurve.cpp
    src/dsp/Resampler.cpp
    src/dsp/FFT.cpp
    src/system/Allocators.cpp
    src/system/sysutils.cpp
    src/system/Thread.cpp
    src/StretcherChannelData.cpp
    src/StretcherImpl.cpp
    src/kissfft/kiss_fft.c
    src/kissfft/kiss_fftr.c
    )
# Convert to list of source files (extension based on OS)
foreach(cpp_file IN LISTS RUBBERBAND_SRC_CPP)
    list(APPEND RUBBERBAND_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rubberband/${cpp_file})
endforeach()

set(RUBBERBAND_HEADERS 
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rubberband/rubberband/RubberBandStretcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rubberband/src/kissfft/kiss_fft.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rubberband/src/kissfft/kiss_fftr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rubberband/src/kissfft/kiss_fft_log.h
)
## Rubberband - end

FILE(GLOB CK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include/util_sndfile.c
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

target_compile_definitions(WarpBufChugin
    PUBLIC
    HAVE_LIBSAMPLERATE
    HAVE_KISSFFT

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
        __MACOSX_CORE__
        USE_PTHREADS
    )   
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC")
else()
    target_compile_definitions(WarpBufChugin
        PUBLIC
        __PLATFORM_LINUX__
        USE_PTHREADS
    )   
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC")
endif()

target_link_libraries(WarpBufChugin samplerate)

# Include header directories
target_include_directories(WarpBufChugin PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Rubberband>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Rubberband/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Rubberband/src/kissfft>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/portable_endian/include>
    $<INSTALL_INTERFACE:WarpBufChugin> )

# Install library
INSTALL(TARGETS WarpBufChugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )

if(MSVC)
target_compile_options(WarpBufChugin PRIVATE /EHsc /GR)
else()
target_compile_options(WarpBufChugin PRIVATE)
endif()

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
else()
  add_custom_command(TARGET ${PROJECT_NAME}
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     "$<TARGET_FILE:WarpBufChugin>"
                     "/usr/local/lib/chuck/WarpBuf.chug")
endif()