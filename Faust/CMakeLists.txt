cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

project(FaucK VERSION 0.0.1)

set(FAUST_DIR ${FAUST_DIR})
set(SndFile_DIR ${SndFile_DIR})
## Create Faust
message(STATUS "FaucK external")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT FaucK)

project(FaucK VERSION 0.0.1)

FILE(GLOB CK_SOURCES ${PROJECT_SOURCE_DIR}/../../chuck/include/*.h)

source_group("ChucK" FILES ${CK_SOURCES})

set(Sources
    "${PROJECT_SOURCE_DIR}/Faust.cpp"
)

source_group("Sources" FILES ${Sources})

set(ALL_FILES
    ${CK_SOURCES}
    ${Sources}
)

add_library(FaucK SHARED ${ALL_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)

find_package(SndFile REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC SndFile::sndfile)

## Link FaucK with libfaust
include_directories(${FAUST_DIR}/architecture)
include_directories(${FAUST_DIR}/compiler)
include_directories(${FAUST_DIR}/compiler/utils)
if(APPLE)
target_link_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/libfaust/macOS-universal)
elseif(WIN32)
target_link_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/libfaust/windows-x86_64)
else()
target_link_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/libfaust/ubuntu-aarch64)
target_link_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/libfaust/ubuntu-x86_64)
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC faust)

if(WIN32)
  # win sock 32
  target_link_libraries(${PROJECT_NAME} ws2_32)

  # windows multimedia for rt midi
  target_link_libraries(${PROJECT_NAME} winmm)
  # more
  target_link_libraries(${PROJECT_NAME} wsock32)
  target_link_libraries(${PROJECT_NAME} dsound dinput8 dxguid)
endif()

if(MSVC)
    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " /ignore:4099 ")
endif()

set(ROOT_NAMESPACE ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/$<CONFIG>/"
    OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/$<CONFIG>/"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)
################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "_USRDLL;"
    "FAUCK_EXPORTS"
)

if(WIN32)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "WIN32;"
    "_WIN32;"
    "_WINDOWS;"
    "_USRDLL;"
    "__WINDOWS_DS__"
    __WINDOWS_MODERN__
    __PLATFORM_WIN32__
)
elseif(APPLE)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "__MACOSX_CORE__;"
    "__APPLE__;"
    "__MACH__;"
)
target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreFoundation" "-framework CoreMIDI" "-framework CoreAudio")
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /Od;
            /RTC1;
            /MDd
        >
        $<$<CONFIG:Release>:
            /MD
        >
        /W3;
        /Zi;
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /DEBUG;
        /SUBSYSTEM:WINDOWS;
        /INCREMENTAL:NO
    )
endif()

target_include_directories(FaucK PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include>
    $<INSTALL_INTERFACE:FaucK> )

if (MSVC)
  add_custom_command(TARGET ${PROJECT_NAME}
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     "$<TARGET_FILE:FaucK>"
                     "%USERPROFILE%/Documents/ChucK/chugins/Faust.chug")

  # # NB: Ideally we would copy the faust.dll next to the chuck.exe,
  # # but Windows doesn't allow this because chuck is inside "C:/Program Files (x86)".
  # # Instead we have to use file explorer to copy it.
  # add_custom_command(TARGET ${PROJECT_NAME}
  #                    POST_BUILD
  #                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
  #                    "$<TARGET_FILE:dynamiclib>"
  #                    "C:/Program\ Files\ (x86)/ChucK/bin/faust.dll")
endif (MSVC)

# Install library
INSTALL(TARGETS FaucK
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )
