cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(rave VERSION 0.0.1)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT rave)

if(MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set(CMAKE_CXX_FLAGS_MINSIZEREL  "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MT")
endif()

# set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(nn_tilde/src/backend) # DEEP LEARNING BACKEND
find_package(Torch REQUIRED)

# list of chuck files
FILE(GLOB CK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include/util_sndfile.c
)

FILE(GLOB NN_TILDE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/nn_tilde/src/backend/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/nn_tilde/src/frontend/maxmsp/shared/*.h
)

set(Sources
        "rave.cpp"
)

source_group("Sources" FILES ${Sources})

add_library(rave SHARED ${CK_SOURCES} ${NN_TILDE_SOURCES} ${Sources})

set_target_properties(rave PROPERTIES CXX_STANDARD 17)

# target_compile_definitions(rave PUBLIC)


if(MSVC)
    target_compile_definitions(rave
    PUBLIC
    USE_BUILTIN_FFT
    NOMINMAX
    _WINDOWS
    WIN32
    __WINDOWS_MODERN__
    __PLATFORM_WIN32__
    __WINDOWS_DS__
   )
endif()


target_link_libraries(rave PRIVATE backend)

# Include header directories
target_include_directories(rave PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../chuck/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/nn_tilde/src/backend>
    $<INSTALL_INTERFACE:rave
)

# Install library
INSTALL(TARGETS rave
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# find all libtorch dlls that eventually need to be copied over
FILE(GLOB Torch_DLLs
	"${Torch_DIR}/../../../lib/*.dll"
)

if (MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES
                        VS_DEBUGGER_COMMAND "C:\\Users\\nick\\Documents\\chuck\\src\\visual-studio\\Debug\\chuck.exe"
                        VS_DEBUGGER_COMMAND_ARGUMENTS "--chugin:build/Debug/rave.chug ../rave_test.ck")

  message(STATUS "in MSVC!")

  # copy chugin over to the relevant chuck dir
  add_custom_command(TARGET ${PROJECT_NAME}
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     "$<TARGET_FILE:rave>"
                     "%USERPROFILE%/Documents/ChucK/chugins/rave.chug")

  # rename to .chug file
  add_custom_command(TARGET ${PROJECT_NAME}
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     "$<TARGET_FILE:rave>"
                     "$<TARGET_FILE_DIR:rave>/rave.chug")

  # copy libtoch dlls to build directory (for unit tests)
  add_custom_command(TARGET ${PROJECT_NAME}  POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${Torch_DLLs}
        $<TARGET_FILE_DIR:rave>)
endif()

############################
### ~~~~~~ Unit Tests ~~~~~~
############################

#[[
set(UNIT_TEST_TARGET_NAME rave_test)
add_library(${UNIT_TEST_TARGET_NAME} SHARED)

target_sources(${UNIT_TEST_TARGET_NAME} PRIVATE 
    rave_test.cpp
)
target_link_libraries(${UNIT_TEST_TARGET_NAME} 
    PRIVATE 
	MSUnitTestFramework::MSUnitTestFramework
	rave
	backend
)
	
enable_testing()

add_library(MSUnitTestFramework::MSUnitTestFramework SHARED IMPORTED)
set_target_properties(MSUnitTestFramework::MSUnitTestFramework PROPERTIES
  # INTERFACE_INCLUDE_DIRECTORIES "$ENV{VCInstallDir}Auxiliary/VS/UnitTest/include"
  # IMPORTED_IMPLIB "$ENV{VCInstallDir}Auxiliary/VS/UnitTest/lib/x86/Microsoft.VisualStudio.TestTools.CppUnitTestFramework.lib"
  IMPORTED_IMPLIB "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/VS/UnitTest/lib/x64/Microsoft.VisualStudio.TestTools.CppUnitTestFramework.lib"
  INTERFACE_INCLUDE_DIRECTORIES "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/VS/UnitTest/include"
)
set(MSUnitTestFramework_FOUND TRUE)
]]
