cmake_minimum_required(VERSION 2.6)
project(TetrisTestSuite)

#
# Detect PLATFORM and ARCH variables
#
if(WIN32)
    set(ARCH x86)
    set(PLATFORM Windows)
elseif(APPLE)
    set(PLATFORM Darwin)
    set(ARCH x86_64)
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
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
else()
set(CMAKE_BUILD_TYPE Release)
endif()


#
# Path variables
#
set(EXECUTABLE TetrisTestSuite)
set(SOLUTIONDIR ${TetrisTestSuite_SOURCE_DIR}/../../)
set(BUILD_ROOT ${SOLUTIONDIR}Build/)
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
# Compile flags
#
if(WIN32)
    set(CXX_FLAGS_Debug   "${CXX_FLAGS} /DTETRIS_QT=1 ")
    set(CXX_FLAGS_Release "${CXX_FLAGS} /DTETRIS_QT=0 ")
    set(FINAL_COMPILE_FLAGS "${CXX_FLAGS_${CMAKE_BUILD_TYPE}} /D POCO_STATIC /D POCO_NO_AUTOMATIC_LIBS /D BOOST_ALL_NO_LIB /D _HAS_ITERATOR_DEBUGGING=0 /D _SECURE_SCL=0 /D _SECURE_SCL_THROWS=0")
else()
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")
    set(CXX_FLAGS         "${CXX_FLAGS} -DTETRIS_QT=1 ")
    set(WARNING_OPTIONS_STM "-std=c++0x -Wall -Wextra -Werror ")
    set(WARNING_OPTIONS     " ${WARNING_OPTIONS_STM}")
    set(CXX_FLAGS_Debug   "${CXX_FLAGS} ${WARNING_OPTIONS} -ggdb3 ")
    set(CXX_FLAGS_Release "${CXX_FLAGS} ${WARNING_OPTIONS} -O3 -DNDEBUG ")
    set(CXX_FLAGS_RelWithDebInfo "${CXX_FLAGS} ${WARNING_OPTIONS} -fno-inline -O1 -g -DNDEBUG ")
    set(FINAL_COMPILE_FLAGS "${CXX_FLAGS_${CMAKE_BUILD_TYPE}}")
endif()


#
# Include directories for Tetris, Futile and Poco.
#
include_directories(
    ${SOLUTIONDIR}Tetris/include
    ${SOLUTIONDIR}Futile/include
    ${SOLUTIONDIR}3rdParty/Boost
    ${SOLUTIONDIR}3rdParty/Poco/Foundation/include
    ${SOLUTIONDIR}3rdParty/GTest/include
    ${SOLUTIONDIR}3rdParty/STM)


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
# Dependencies
#
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTDIR}lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTDIR}bin/)
add_subdirectory(${SOLUTIONDIR}Tetris ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Tetris)
add_subdirectory(${SOLUTIONDIR}Futile ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Futile)


#
# Testsuite source files
#
set(TetrisTestSuite_SOURCES
    src/main.cpp
    src/ComputerPlayerTest.cpp
    src/NodeCalculatorTest.cpp
    src/STMTest.cpp
    src/TetrisTest.cpp
    src/WorkerPoolTest.cpp
    src/WorkerTest.cpp)


#
# Testsuite header files
#
set(TetrisTestSuite_HEADERS
    src/NodeCalculatorTest.h
    src/TetrisTest.h
    src/WorkerPoolTest.h
    src/WorkerTest.h)


#
# Set the executable
#
add_executable(${EXECUTABLE} ${TetrisTestSuite_SOURCES})


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
# Lib filename extension
#
set(LIBEXT "a")
if(WIN32)
set(LIBEXT "lib")
endif()


#
# Testsuite libs
#
target_link_libraries(${EXECUTABLE}
    Tetris
    Futile
    PocoFoundation
    CppUnit
    pthread
    ${SOLUTIONDIR}3rdParty/Boost/lib/${PLATFORM}/${ARCH}/libboost_thread.${LIBEXT}
    ${SOLUTIONDIR}3rdParty/Poco/lib/${PLATFORM}/${ARCH}/libPocoFoundation.${LIBEXT}
    ${SOLUTIONDIR}3rdParty/GTest/lib/${PLATFORM}/${ARCH}/libgtest.${LIBEXT})


#
# Target properties
#
set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS ${FINAL_COMPILE_FLAGS})
