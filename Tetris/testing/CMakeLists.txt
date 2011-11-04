cmake_minimum_required(VERSION 2.6)
project(TetrisTest)


#
# Check TETRIS_BUILD_TYPE environment variable
#
set(CMAKE_BUILD_TYPE $ENV{TETRIS_BUILD_TYPE})
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Release")
message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
else()
set(CMAKE_BUILD_TYPE Release)
endif()


#
# Detect PLATFORM and ARCH variables
#
if(WIN32)
    set(PLATFORM Windows)
    set(ARCH x86)
else()
    execute_process(COMMAND uname OUTPUT_VARIABLE PLATFORM)
    string(REPLACE "\n" "" PLATFORM ${PLATFORM})
    execute_process(COMMAND arch OUTPUT_VARIABLE ARCH)
    string(REPLACE "\n" "" ARCH ${ARCH})
endif()


message("PLATFORM: ${PLATFORM}")
message("ARCH:     ${ARCH}")


#
# Path variables
#
set(EXECUTABLE TetrisTest)
set(CMAKE_OSX_ARCHITECTURES ${ARCH})
set(SOLUTIONDIR "${TetrisTest_SOURCE_DIR}/../../")
set(BUILD_ROOT ${SOLUTIONDIR}Build/)


#
# Set output directory variable
#
set(OUTDIR ${BUILD_ROOT}${CMAKE_BUILD_TYPE}/${PLATFORM}/${ARCH}/)


#
# Now we can set the CMAKE vars
#
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTDIR}lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTDIR}bin/)


#
# Compile flags
#
set(WARNING_OPTIONS         " -Wall -Werror ")
set(CXX_FLAGS_Debug   "${CXX_FLAGS} ${WARNING_OPTIONS} -ggdb3")
set(CXX_FLAGS_Release "${CXX_FLAGS} ${WARNING_OPTIONS} -O3 -DNDEBUG")
set(FINAL_COMPILE_FLAGS "${CXX_FLAGS_${CMAKE_BUILD_TYPE}}")


#
# Platform specific compiler flags
#
if(APPLE)
    set(WARNING_OPTIONS "-arch ${ARCH} ")
endif()


#
# Extra platform specific libs
#
if(WIN32)
    set(EXTRA_LIBS "-lIphlpapi ${EXTRA_LIBS}")
else()
    set(EXTRA_LIBS "pthread")
endif()


#
# Poco dependency
#
set(POCO_ROOT "${SOLUTIONDIR}3rdParty/Poco/")
include_directories(
    ${POCO_ROOT}Foundation/include
    ${POCO_ROOT}CppUnit/include)
set(POCO_LIB "${SOLUTIONDIR}3rdParty/Poco/lib/${PLATFORM}/${ARCH}")
link_directories(${POCO_LIB})


#
# Boost dependency
#
set(BOOST_ROOT "${SOLUTIONDIR}3rdParty/Boost")
include_directories(${BOOST_ROOT})
set(BOOST_LIB "${BOOST_ROOT}/lib/${PLATFORM}/${ARCH}")


#
# Tetris & Futile dependencies
#
add_subdirectory(${SOLUTIONDIR}Tetris ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Tetris)
add_subdirectory(${SOLUTIONDIR}Futile ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Futile)
include_directories(
    ${SOLUTIONDIR}Futile/include
    ${SOLUTIONDIR}Tetris/include)


#
# TetrisTest source files
#
set(TetrisTest_SOURCES
    src/Driver.cpp
    src/NodeCalculatorTest.cpp
    src/TetrisTest.cpp
    src/WorkerPoolTest.cpp
    src/WorkerTest.cpp)


#
# TetrisTest header files
#
set(TetrisTest_HEADERS 
    src/NodeCalculatorTest.h
    src/TetrisTest.h
    src/WorkerPoolTest.h
    src/WorkerTest.h)



#
# Platform specific options for the creation of the executable
#
if(APPLE)
    set(EXECUTABLE_OPTIONS MACOSX_BUNDLE)
else()
    set(EXECUTABLE_OPTIONS "")
endif()


#
# Set the executable. On Mac we want a bundle.
#
add_executable(${EXECUTABLE} ${EXECUTABLE_OPTIONS} ${TetrisTest_SOURCES} ${TetrisTest_HEADERS_MOC} ${TetrisTest_Resources})


#
# Post build step: make a symbolic link to the last build directory
#
if(WIN32)
# TODO: Add unix shell support. Or use CMake builtin commands.
else()
add_custom_command(TARGET ${EXECUTABLE}
                   POST_BUILD
                   COMMAND /bin/rm ARGS -f ${BUILD_ROOT}LastBuild
                   COMMAND /bin/ln ARGS -s ${OUTDIR} ${BUILD_ROOT}LastBuild
                   WORKING_DIRECTORY    ${BUILD_ROOT})
endif()


#
# TetrisTest libs
#
target_link_libraries(${EXECUTABLE}
    Tetris
    Futile
    PocoFoundation
    ${EXTRA_LIBS}
    "${BOOST_LIB}/libboost_thread.a"
    "${BOOST_LIB}/libboost_signals.a")


#
# Target properties
#
set_target_properties(${EXECUTABLE}  PROPERTIES
                      COMPILE_FLAGS  ${FINAL_COMPILE_FLAGS})

