cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

project(FaucK VERSION 0.0.1)

set(FAUST_DIR ${FAUST_DIR})
include(FaustLib.cmake)
## Create Faust
message(STATUS "FaucK external")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT FaucK)

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"))
    message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
endif()

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

## Link FaucK with faustlib
include_directories(${FAUST_DIR}/architecture)
add_dependencies(${PROJECT_NAME} dynamiclib)
target_link_libraries(${PROJECT_NAME} dynamiclib)

## Link the Pure Data external with llvm
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

if(WIN32)
  # win sock 32
  target_link_libraries(${PROJECT_NAME} ws2_32)
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
    "WIN32;"
    "_WIN32;"
    "_WINDOWS;"
    "_USRDLL;"
    "__WINDOWS_DS__"
    __WINDOWS_MODERN__
    __PLATFORM_WIN32__
    "FAUCK_EXPORTS"
)

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
