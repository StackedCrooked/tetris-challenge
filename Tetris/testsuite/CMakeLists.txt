cmake_minimum_required(VERSION 2.6)
project(TetrisTestSuite)

FIND_PACKAGE(Qt4 REQUIRED)


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
# Detect the build configuration.
# Reads from environment variable TETRIS_BUILD_CONFIGURATION
#
set(CMAKE_BUILD_TYPE $ENV{TETRIS_BUILD_CONFIGURATION})
message("TETRIS_BUILD_CONFIGURATION: $ENV{TETRIS_BUILD_CONFIGURATION}")


#
# Path variables
#
set(EXECUTABLE TetrisTestSuite)
set(CMAKE_OSX_ARCHITECTURES ${ARCH})
set(SOLUTIONDIR "${TetrisTestSuite_SOURCE_DIR}/../../")
set(POCO ${SOLUTIONDIR}3rdParty/Poco/)


#
# Set output directory
#
# Note:
# If you want to build multiple platforms/architetures from the same
# location then you need to use the second definition below.
#
set(OUTDIR ${SOLUTIONDIR}Build/)
#set(OUTDIR ${SOLUTIONDIR}Build/${PLATFORM}/${ARCH}/${CMAKE_BUILD_TYPE}/)


#
# Compile Flags
#
set(WARNING_OPTIONS         "-arch ${ARCH} -Wall -Wno-sign-compare")
set(WARNING_OPTIONS_Debug   "${WARNING_OPTIONS} -arch ${ARCH} -Werror ")
set(WARNING_OPTIONS_Release "${WARNING_OPTIONS} -arch ${ARCH} ")
set(CXX_FLAGS         "${CXX_FLAGS} -DTETRIS_QT=1")
set(CXX_FLAGS_Debug   "${CXX_FLAGS} ${WARNING_OPTIONS_Debug} -ggdb3")
set(CXX_FLAGS_Release "${CXX_FLAGS} ${WARNING_OPTIONS_Release} -O3 -DNDEBUG")
set(FINAL_COMPILE_FLAGS "${CXX_FLAGS_${CMAKE_BUILD_TYPE}}")


#
# Poco dependency
#
include_directories(${POCO}Foundation/include ${POCO}CppUnit/include ${SOLUTIONDIR}Tetris/include)
link_directories(${POCO}lib/${PLATFORM}/${ARCH})


#
# Boost dependency
#
if(APPLE)
message("Note: MacOSX build depends on boost library from MacPorts!")
include_directories("/opt/local/include")
link_directories("/opt/local/lib")
set(LIB_BOOST_THREAD boost_thread-mt)
else()
set(BOOST ${SOLUTIONDIR}3rdParty/Boost_1_44_0/)
include_directories(${BOOST})
link_directories(${BOOST}lib/${PLATFORM}/${ARCH})
set(LIB_BOOST_THREAD boost_thread)
endif()


#
# Tetris dependency
#
add_subdirectory(${SOLUTIONDIR}Tetris ${OUTDIR}Tetris)


#
# Testsuite source files
#
set(TetrisTestSuite_SOURCES
    src/Driver.cpp
    src/GenericGridTest.cpp
    src/NodeCalculatorTest.cpp
    src/TetrisTestSuite.cpp
    src/WorkerPoolTest.cpp
    src/WorkerTest.cpp)


#
# Testsuite header files
#
set(TetrisTestSuite_HEADERS
    src/GenericGridTest.h
    src/NodeCalculatorTest.h
    src/TetrisTestSuite.h
    src/WorkerPoolTest.h
    src/WorkerTest.h)


#
# Special settings to make CMake and Qt work together
#
QT4_WRAP_CPP(TetrisTestSuite_HEADERS_MOC ${TetrisTestSuite_HEADERS})
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})


#
# Set the executable
#
add_executable(${EXECUTABLE} ${TetrisTestSuite_SOURCES} ${TetrisTestSuite_HEADERS_MOC})


#
# Testsuite libs
#
target_link_libraries(${EXECUTABLE}
    ${QT_LIBRARIES}
    Tetris
    PocoFoundation
    CppUnit
    ${LIB_BOOST_THREAD})


#
# Target properties
#
set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS ${FINAL_COMPILE_FLAGS})
