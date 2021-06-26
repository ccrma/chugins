cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

project(VST VERSION 0.0.1)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VST)

set(CMAKE_VS_PLATFORM_NAME "x64")

################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)

################################################################################
# Global compiler options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")

endif()

set(cxx_no_exception_flags "-EHs-c- -D_HAS_EXCEPTIONS=1")

################################################################################
# Nuget packages function stub.
################################################################################
function(use_package TARGET PACKAGE VERSION)
    message(WARNING "No implementation of use_package. Create yours. "
                    "Package \"${PACKAGE}\" with version \"${VERSION}\" "
                    "for target \"${TARGET}\" is ignored!")
endfunction()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Sub-projects
################################################################################
project(VST VERSION 0.0.1)

set(JUCE_MODULES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/JUCE_5/modules")

message(STATUS "Using JUCE module path: ${JUCE_MODULES_PATH}")

# Default extension for source files
if(UNIX AND APPLE)
  set(SOURCE_EXTENSION "mm")
else ()
  set(SOURCE_EXTENSION "cpp")
endif()

# List of modules to build
set(JUCE_MODULES
    juce_audio_basics
    juce_audio_devices
    juce_audio_formats
    juce_audio_processors
    juce_audio_utils
    juce_core
    juce_cryptography
    juce_data_structures
    juce_dsp
    juce_events
    juce_graphics
    juce_gui_basics
    juce_gui_extra
    juce_opengl
    juce_video)
# Convert to list of source files (extension based on OS)
foreach(j_module IN LISTS JUCE_MODULES)
    list(APPEND JUCE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/include_${j_module}.${SOURCE_EXTENSION} )
endforeach()

list(APPEND JUCE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/AppConfig.h")
list(APPEND JUCE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/JuceHeader.h")

foreach(j_module IN LISTS JUCE_MODULES)
    list(APPEND JUCE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/include_${j_module}.${SOURCE_EXTENSION} )
endforeach()

FILE(GLOB CK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include/*.h)

set(Headers
    "JuceLibraryCode/AppConfig.h"
    "JuceLibraryCode/JuceHeader.h"
)
source_group("Headers" FILES ${Headers})

set(Sources
    "src/VST.cpp"
)

source_group("Sources" FILES ${Sources})

add_library(VST SHARED ${JUCE_SOURCES} ${CK_SOURCES} ${Headers} ${Sources})
target_compile_definitions(VST
    PUBLIC
        JUCE_STANDALONE_APPLICATION=1
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DLL=1
        JUCE_DLL_BUILD=1

        _WINDOWS
        WIN32
        __WINDOWS_MODERN__
        __PLATFORM_WIN32__
        __WINDOWS_DS__
        _USRDLL

        Use_MFC=0
        Use_Debug_Libraries=0
        NDEBUG
        _USRDLL
        
        _MBCS
        VST_EXPORTS
)

# Include header directories
target_include_directories(VST PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode>
    $<BUILD_INTERFACE:${JUCE_MODULES_PATH}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/JUCE_5/modules/juce_audio_processors/format_types/VST3_SDK>
    $<INSTALL_INTERFACE:VST> )

# Install library
INSTALL(TARGETS VST
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )

# TODO: NEED -O3 FLAG
target_compile_options(VST PRIVATE /EHsc /GR)

if(MSVC)
    target_compile_options(VST PRIVATE
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
    target_link_options(VST PRIVATE
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
  add_custom_command(TARGET ${PROJECT_NAME}
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     "$<TARGET_FILE:VST>"
                     "%USERPROFILE%/Documents/ChucK/chugins/VST.chug")
endif (MSVC)
