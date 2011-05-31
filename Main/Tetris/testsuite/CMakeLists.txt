cmake_minimum_required(VERSION 2.6)
project(TetrisTestSuite)

#
# Detect PLATFORM and ARCH variables
#
if(WIN32)
    set(ARCH x86)
    set(PLATFORM Windows)
else()
    execute_process(COMMAND uname OUTPUT_VARIABLE PLATFORM)
    string(REPLACE "\n" "" PLATFORM ${PLATFORM})
    execute_process(COMMAND arch OUTPUT_VARIABLE ARCH)
    string(REPLACE "\n" "" ARCH ${ARCH})
endif()


#
# Check TETRIS_BUILD_TYPE environment variable
#
set(CMAKE_BUILD_TYPE $ENV{TETRIS_BUILD_TYPE})
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Release")
    message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
else()
    set(CMAKE_BUILD_TYPE "Release")
    message("TETRIS_BUILD_TYPE was not set or not correctly set. Defaulting to \"Release\"")
else()
endif()


#
# Path variables
#
set(EXECUTABLE TetrisTestSuite)
set(CMAKE_OSX_ARCHITECTURES ${ARCH})
set(SOLUTIONDIR ${TetrisTestSuite_SOURCE_DIR}/../../)
set(BUILD_ROOT ${SOLUTIONDIR}../Build/)
set(POCO ${SOLUTIONDIR}3rdParty/Poco/)


#
# Set output directory
#
set(OUTDIR ${BUILD_ROOT}${CMAKE_BUILD_TYPE}/${PLATFORM}/${ARCH}/)


#
# Now we can set the CMAKE vars
#
set(LIBRARY_OUTPUT_PATH ${OUTDIR}lib/)
set(EXECUTABLE_OUTPUT_PATH ${OUTDIR}bin/)


#
# Compile Flags
#
set(WARNING_OPTIONS         "-arch ${ARCH} ")
set(WARNING_OPTIONS_Debug   "${WARNING_OPTIONS} -Wall -Werror -Wno-sign-compare")
set(CXX_FLAGS         "${CXX_FLAGS} -pthread ")
set(CXX_FLAGS_Debug   "${CXX_FLAGS} ${WARNING_OPTIONS_Debug} -ggdb3 ")
set(CXX_FLAGS_Release "${CXX_FLAGS} ${WARNING_OPTIONS_Release} -O3 -DNDEBUG ")
set(FINAL_COMPILE_FLAGS "${CXX_FLAGS_${CMAKE_BUILD_TYPE}}")


#
# Include directories for Tetris, Futile and Poco.
#
include_directories(
    ${POCO}Foundation/include
    ${POCO}CppUnit/include
    ${SOLUTIONDIR}Futile/include
    ${SOLUTIONDIR}Tetris/include)


#
# Link with Poco
#
link_directories(${POCO}lib/${PLATFORM}/${ARCH})


#
# Boost dependency
#
set(BOOST_ROOT ${SOLUTIONDIR}3rdParty/Boost)
include_directories(${BOOST_ROOT})
set(BOOST_LIB "${BOOST_ROOT}/lib/${PLATFORM}/${ARCH}")


#
# Add Tetris project to our build
#
add_subdirectory(${SOLUTIONDIR}Tetris ${LIBRARY_OUTPUT_PATH}/Tetris)
add_subdirectory(${SOLUTIONDIR}Futile ${LIBRARY_OUTPUT_PATH}/Futile)


#
# Testsuite source files
#
set(TetrisTestSuite_SOURCES
    src/Driver.cpp
    src/GenericGridTest.cpp
    src/MemoryPoolTest.cpp
    src/NodeCalculatorTest.cpp
    src/TetrisTestSuite.cpp
    src/WorkerPoolTest.cpp
    src/WorkerTest.cpp)


#
# Testsuite header files
#
set(TetrisTestSuite_HEADERS
    src/GenericGridTest.h
    src/MemoryPoolTest.h
    src/NodeCalculatorTest.h
    src/TetrisTestSuite.h
    src/WorkerPoolTest.h
    src/WorkerTest.h)


#
# Set the executable
#
add_executable(${EXECUTABLE} ${TetrisTestSuite_SOURCES} ${TetrisTestSuite_HEADERS_MOC})


#
# Testsuite libs
#
target_link_libraries(${EXECUTABLE}
    Tetris
    Futile
    PocoFoundation
    CppUnit
    "${BOOST_LIB}/libboost_thread.a")


#
# Target properties
#
set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS ${FINAL_COMPILE_FLAGS})

