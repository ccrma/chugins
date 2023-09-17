cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set (VERSION "0.0.1")

project(FaucK VERSION ${VERSION})

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


#######################################
# faust libraries
file (GLOB DSPLIBS ${LIBFAUST_DIR}/share/faust/*.lib)

# Create shared library
add_library(FaucK MODULE ${ALL_FILES} ${DSPLIBS})

# set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

####################################
# Install - used for packaging
####################################
set (PACKDIR ${ROOT}/package)
set (DST fauck-${VERSION})
install ( TARGETS FaucK
    RUNTIME DESTINATION ${PACKDIR}/${DST}
    LIBRARY DESTINATION ${PACKDIR}/${DST}
    ARCHIVE DESTINATION ${PACKDIR}/${DST}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(APPLE)
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINK_FLAGS "-Wl,-U,_object_new_imp -Wl,-U,_object_method_imp"
    BUNDLE TRUE
    BUNDLE_EXTENSION chug
    OUTPUT_NAME Faust
    RESOURCE "${DSPLIBS}"
    MACOSX_BUNDLE_GUI_IDENTIFIER edu.stanford.chuck.${PROJECT_NAME}
    MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
    MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION}
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist
)
else()
set_target_properties (${PROJECT_NAME} PROPERTIES 
    SUFFIX ".chug"
    OUTPUT_NAME Faust)
install (
    FILES ${DSPLIBS} DESTINATION ${PACKDIR}/${DST}/faustlibraries
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)
endif()

# Find sndfile and link it
if (MSVC)
    find_package(SndFile REQUIRED HINTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsndfile-1.2.0-win64/cmake")
    target_link_libraries(${PROJECT_NAME} PRIVATE SndFile::sndfile)
else()
    find_package(PkgConfig REQUIRED)
    # We expect the user to have used brew to install the dependencies
    # for libsndfile, to have built libsndfile as a static lib,
    # and for it to be accessible via `pkg-config --static libsndfile`.
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    list(APPEND PKG_CONFIG_EXECUTABLE "--static")  # append temporarily
    pkg_check_modules(SNDFILE REQUIRED IMPORTED_TARGET sndfile)
    pkg_check_modules(FLAC REQUIRED IMPORTED_TARGET flac)
    pkg_check_modules(VORBIS REQUIRED IMPORTED_TARGET vorbis)
    pkg_check_modules(OGG REQUIRED IMPORTED_TARGET ogg)
    pkg_check_modules(OPUS REQUIRED IMPORTED_TARGET opus)
    pkg_check_modules(MPG123 REQUIRED IMPORTED_TARGET libmpg123)
    if(APPLE)
    execute_process(COMMAND find /usr/local/Cellar/lame -name libmp3lame.a OUTPUT_VARIABLE SNDFILE_STATIC_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
    else()
    execute_process(COMMAND find /usr/local -name libmp3lame.a OUTPUT_VARIABLE SNDFILE_STATIC_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
    list(POP_BACK PKG_CONFIG_EXECUTABLE)  # undo the append above
    target_link_libraries (${PROJECT_NAME} PRIVATE PkgConfig::SNDFILE PkgConfig::FLAC PkgConfig::VORBIS PkgConfig::OGG PkgConfig::OPUS PkgConfig::MPG123 ${SNDFILE_STATIC_LIBS})
endif()

# Include directories for libfaust
if(NOT FAUST_DIR)
    set(FAUST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/faust")
endif()
include_directories(${FAUST_DIR}/architecture)
include_directories(${FAUST_DIR}/compiler)
include_directories(${FAUST_DIR}/compiler/utils)
unset(FAUST_DIR CACHE)

# Link libfaust based on platform
target_link_directories(${PROJECT_NAME} PRIVATE ${LIBFAUST_DIR}/lib)
if(MSVC)
target_link_libraries(${PROJECT_NAME} PRIVATE libfaustwithllvm.lib)
else()
target_link_libraries(${PROJECT_NAME} PRIVATE libfaustwithllvm.a)
endif()

# Platform-specific libraries and definitions
if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "__MACOSX_CORE__;__APPLE__;__PLATFORM_APPLE__")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreFoundation" "-framework CoreMIDI" "-framework CoreAudio")
elseif(MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 winmm wsock32 dsound dinput8 dxguid)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "WIN32;_WIN32;_WINDOWS;NOMINMAX;_USRDLL;__WINDOWS_DS__;__PLATFORM_WINDOWS__")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE "__PLATFORM_LINUX__")
    # target_compile_options(${PROJECT_NAME} PUBLIC "-std=c++17")
    # target_link_libraries(${PROJECT_NAME} PUBLIC "stdc++fs")
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

if (WIN32)
####################################
# install sndfile
install (
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsndfile-1.2.0-win64/bin/sndfile.dll DESTINATION ${PACKDIR}/${DST}
)
endif()

if (APPLE)
install (
    CODE "execute_process (COMMAND codesign --sign - --deep --force ${PACKDIR}/${DST}/Faust.chug)" 
    )
endif()
