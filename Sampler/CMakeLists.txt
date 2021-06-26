cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

project(SamplerChugin VERSION 0.0.1)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SamplerChugin)

set(CMAKE_VS_PLATFORM_NAME "x64")

project(SamplerChugin VERSION 0.0.1)

FILE(GLOB CK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include/*.h)

file (GLOB SAMPLER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Sampler/Source/*.h"
                           "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Sampler/Source/*.cpp"
                           "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Sampler/Source/Components/*.h"      
                           "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Sampler/Source/Components/*.cpp"
                           "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Sampler/Source/DataModels/*.h"
                           "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Sampler/Source/DataModels/*.cpp")

set(Sources
    "src/SamplerChugin.cpp"
)

source_group("Sources" FILES ${Sources})

add_library(SamplerChugin SHARED ${CK_SOURCES} ${Sources} ${SAMPLER_SOURCES})

add_subdirectory(thirdparty/JUCE)

target_link_libraries(SamplerChugin
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        # juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core    
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

target_compile_definitions(SamplerChugin
    PUBLIC
        # JUCE_DLL=1
        # JUCE_STANDALONE_APPLICATION=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        _WINDOWS
        WIN32
        __WINDOWS_MODERN__
        __PLATFORM_WIN32__
        __WINDOWS_DS__
        _USRDLL
        Use_MFC=0
        Use_Debug_Libraries=0
        NDEBUG
        _NDEBUG
        _MBCS
        SamplerChugin_EXPORTS

        JUCE_STANDALONE_APPLICATION=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        # JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
        JUCE_MODULE_AVAILABLE_juce_audio_basics=1
        JUCE_MODULE_AVAILABLE_juce_audio_devices=1
        JUCE_MODULE_AVAILABLE_juce_audio_formats=1
        # JUCE_MODULE_AVAILABLE_juce_audio_plugin_client=1
        JUCE_MODULE_AVAILABLE_juce_audio_processors=1
        JUCE_MODULE_AVAILABLE_juce_audio_utils=1
        JUCE_MODULE_AVAILABLE_juce_core=1
        JUCE_MODULE_AVAILABLE_juce_data_structures=1
        JUCE_MODULE_AVAILABLE_juce_dsp=1
        JUCE_MODULE_AVAILABLE_juce_events=1
        JUCE_MODULE_AVAILABLE_juce_graphics=1
        JUCE_MODULE_AVAILABLE_juce_gui_basics=1
        JUCE_MODULE_AVAILABLE_juce_gui_extra=1

        PIP_JUCE_EXAMPLES_DIRECTORY=QzpcdG9vbHNcSlVDRVxleGFtcGxlcw==
        JucePlugin_Build_VST=0
        JucePlugin_Build_VST3=0
        JucePlugin_Build_AU=0
        JucePlugin_Build_AUv3=0
        JucePlugin_Build_RTAS=0
        JucePlugin_Build_AAX=0
        JucePlugin_Build_Standalone=0
        JucePlugin_Build_Unity=0
        _LIB
    # PRIVATE
    #     JUCE_DLL_BUILD=1
)

if(MSVC)
    target_compile_definitions(SamplerChugin
        PUBLIC
            _WINDOWS
            WIN32
            __WINDOWS_MODERN__
            __PLATFORM_WIN32__
            __WINDOWS_DS__
            )
endif()

# Include header directories
target_include_directories(SamplerChugin PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Sampler/JuceLibraryCode>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Sampler/Source/>
    $<INSTALL_INTERFACE:SamplerChugin> )

# Install library
INSTALL(TARGETS SamplerChugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )

target_compile_options(SamplerChugin PRIVATE /EHsc /GR)

if(MSVC)
    target_compile_options(SamplerChugin PRIVATE
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
    target_link_options(SamplerChugin PRIVATE
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
                     "$<TARGET_FILE:SamplerChugin>"
                     "%USERPROFILE%/Documents/ChucK/chugins/Sampler.chug")
endif (MSVC)
