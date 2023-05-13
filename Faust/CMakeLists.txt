cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

project(FaucK VERSION 0.0.1)

# Set startup project for Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT FaucK)

# Collect ChucK sources
FILE(GLOB CK_SOURCES ${PROJECT_SOURCE_DIR}/../../chuck/include/*.h)
source_group("ChucK" FILES ${CK_SOURCES})

# Collect FaucK sources
set(Sources
    "${PROJECT_SOURCE_DIR}/Faust.cpp"
)
source_group("Sources" FILES ${Sources})

# Combine all sources
set(ALL_FILES
    ${CK_SOURCES}
    ${Sources}
)

# Create shared library
add_library(FaucK SHARED ${ALL_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Find sndfile and link it
find_package(SndFile REQUIRED HINTS ${SndFile_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE SndFile::sndfile)

# Include directories for libfaust
include_directories(${FAUST_DIR}/architecture)
include_directories(${FAUST_DIR}/compiler)
include_directories(${FAUST_DIR}/compiler/utils)

# Link libfaust based on platform
target_link_directories(${PROJECT_NAME} PRIVATE ${LIBFAUST_DIR}/lib)
if(MSVC)
target_link_libraries(${PROJECT_NAME} PRIVATE libfaustwithllvm.lib)
else()
target_link_libraries(${PROJECT_NAME} PRIVATE libfaustwithllvm.a)
endif()

# Platform-specific libraries and definitions
if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "__MACOSX_CORE__;__APPLE__")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreFoundation" "-framework CoreMIDI" "-framework CoreAudio")
elseif(MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 winmm wsock32 dsound dinput8 dxguid)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "WIN32;_WIN32;_WINDOWS;NOMINMAX;_USRDLL;__WINDOWS_DS__;__WINDOWS_MODERN__;__PLATFORM_WIN32__")
endif()

# MSVC-specific options
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Debug>:_DEBUG>$<$<CONFIG:Release>:NDEBUG>")
    # target_compile_options(${PROJECT_NAME} PRIVATE "/W3;/Zi;${DEFAULT_CXX_EXCEPTION_HANDLING}")
    target_link_options(${PROJECT_NAME} PRIVATE "/DEBUG;/SUBSYSTEM:WINDOWS;/INCREMENTAL:NO")
endif()

# Debug and Release configurations
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "_USRDLL;"
    "FAUCK_EXPORTS"
)

# Include directories
target_include_directories(FaucK PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include>
    $<INSTALL_INTERFACE:FaucK> )

# Copy library to ChucK chugins directory
if (MSVC)
  add_custom_command(TARGET ${PROJECT_NAME}
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     "$<TARGET_FILE:FaucK>"
                     "%USERPROFILE%/Documents/ChucK/chugins/Faust.chug")
endif (MSVC)

# Install library
INSTALL(TARGETS FaucK
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )