cmake_minimum_required(VERSION 2.6) 
project(TetrisTestSuite) 

set(CONFIGURATION Release)

if(APPLE)
set(ARCH i386)
set(PLATFORM Darwin)
elseif(UNIX)
set(ARCH x86_64)
set(PLATFORM Linux)
elseif(WIN32)
set(ARCH x86)
set(PLATFORM Windows)
endif()

set(EXECUTABLE tetristestsuite)
set(SOLUTIONDIR ${TetrisTestSuite_SOURCE_DIR}/)
set(OUTDIR ${SOLUTIONDIR}Build/${PLATFORM}/${ARCH}/${CONFIGURATION}/)
set(BOOST ${SOLUTIONDIR}3rdParty/Boost_1_44_0/)
set(POCO ${SOLUTIONDIR}3rdParty/Poco/)


message("CONFIGURATION: ${CONFIGURATION}")

if(${CONFIGURATION} STREQUAL Release)
set(CXX_FLAGS "${CXX_FLAGS} -Wall -O3")
elseif(${CONFIGURATION} STREQUAL Debug)
set(CXX_FLAGS "${CXX_FLAGS} -Wall -ggdb")
endif()


include_directories(${BOOST}
                    ${POCO}/Foundation/include
                    ${POCO}/CppUnit/include)

link_directories(${BOOST}lib/${PLATFORM}/${ARCH})
link_directories(${POCO}lib/${PLATFORM}/${ARCH})



add_subdirectory(${SOLUTIONDIR}TetrisCore ${OUTDIR}TetrisCore)
add_subdirectory(${SOLUTIONDIR}TetrisCore/testsuite ${OUTDIR}TetrisTestSuite)

