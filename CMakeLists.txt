cmake_minimum_required(VERSION 2.6)
project(TETRIS)

set(CMAKE_BUILD_TYPE $ENV{TETRIS_BUILD_CONFIGURATION})
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    message("Defaulting to \"Release\" since the environment variable TETRIS_BUILD_CONFIGURATION was not set.")
    set(CMAKE_BUILD_TYPE "Release")
else()
    message("Detected build configuration from environment variable TETRIS_BUILD_CONFIGURATION: ${CMAKE_BUILD_TYPE}.")
endif()

message("\n\n***")
message("===> Build configuration is: ${CMAKE_BUILD_TYPE} <===")
message("***\n\n")

if(WIN32)
    set(ARCH x86)
    set(PLATFORM Windows)
else()
    execute_process(COMMAND uname OUTPUT_VARIABLE PLATFORM)
    string(REPLACE "\n" "" PLATFORM ${PLATFORM})
    execute_process(COMMAND arch OUTPUT_VARIABLE ARCH)
    string(REPLACE "\n" "" ARCH ${ARCH})
endif()


set(EXECUTABLE testsuite)
set(SOLUTIONDIR ${TETRIS_SOURCE_DIR}/)
set(OUTDIR ${SOLUTIONDIR}Build/${PLATFORM}/${ARCH}/${CMAKE_BUILD_TYPE}/)
set(BOOST ${SOLUTIONDIR}3rdParty/Boost_1_44_0/)
set(POCO ${SOLUTIONDIR}3rdParty/Poco/)
set(WARNING_OPTIONS "-Wall -Werror -Wno-sign-compare")
set(CXX_FLAGS_Release "${CXX_FLAGS} ${WARNING_OPTIONS} -O3")
set(CXX_FLAGS_Debug   "${CXX_FLAGS} ${WARNING_OPTIONS} -ggdb")
set(FINAL_COMPILE_FLAGS "${CXX_FLAGS_${CMAKE_BUILD_TYPE}}")


include_directories(
    ${BOOST} ${POCO}/Foundation/include
    ${POCO}/CppUnit/include)


link_directories(
    ${BOOST}lib/${PLATFORM}/${ARCH}
    ${POCO}lib/${PLATFORM}/${ARCH})


add_subdirectory(${SOLUTIONDIR}Tetris ${OUTDIR}Tetris)
add_subdirectory(${SOLUTIONDIR}Tetris/testsuite ${OUTDIR}TetrisTestSuite)

