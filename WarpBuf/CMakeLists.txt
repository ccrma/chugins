cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

project(WarpBufChugin VERSION 0.0.1)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT WarpBufChugin)

set(CMAKE_VS_PLATFORM_NAME "x64")

project(WarpBufChugin VERSION 0.0.1)

## libsamplerate - start

# list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/cmake)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate)
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
    # ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rubberband/rubberband/rubberband-c.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rubberband/rubberband/RubberBandStretcher.h
)
## Rubberband - end

FILE(GLOB CK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include/util_sndfile.c
    )

# FILE(GLOB LIBSAMPLERATE_SOURCES
#     # ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/src/samplerate.c
#     ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/src/*.h)

set(Sources
    "src/WarpBufChugin.cpp"
)

source_group("Sources" FILES ${Sources})

add_library(WarpBufChugin SHARED ${CK_SOURCES} ${Sources} ${RUBBERBAND_SOURCES} ${RUBBERBAND_HEADERS}
    # ${LIBSAMPLERATE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/include/samplerate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/portable_endian/include/portable_endian.h
    )

target_compile_definitions(WarpBufChugin
    PUBLIC
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
            HAVE_LIBSAMPLERATE
            USE_KISSFFT
            NOMINMAX

            _WINDOWS
            WIN32
            __WINDOWS_MODERN__
            __PLATFORM_WIN32__
            __WINDOWS_DS__
            )
endif()

target_link_libraries(WarpBufChugin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/build/src/Release/samplerate.lib)

# Include header directories
target_include_directories(WarpBufChugin PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Rubberband>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Rubberband/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsamplerate/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/portable_endian/include>
    # $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Sampler/Source/>
    $<INSTALL_INTERFACE:WarpBufChugin> )

# Install library
INSTALL(TARGETS WarpBufChugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )

target_compile_options(WarpBufChugin PRIVATE /EHsc /GR)

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
endif (MSVC)