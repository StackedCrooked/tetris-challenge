cmake_minimum_required(VERSION 2.6) 
project(TETRIS) 


set(CMAKE_BUILD_TYPE Release)


if(APPLE)
	set(ARCH i386)
	set(PLATFORM Darwin)
	
elseif(CYGWIN)
	set(ARCH i686)
	set(PLATFORM Cygwin)
	
elseif(UNIX)
	set(ARCH x86_64)
	set(PLATFORM Linux)
	
elseif(WIN32)
	set(ARCH x86)
	set(PLATFORM Windows)
endif()


set(EXECUTABLE testsuite)
set(SOLUTIONDIR ${TETRIS_SOURCE_DIR}/)
set(OUTDIR ${SOLUTIONDIR}Build/${PLATFORM}/${ARCH}/${CMAKE_BUILD_TYPE}/)
set(BOOST ${SOLUTIONDIR}3rdParty/Boost_1_44_0/)
set(POCO ${SOLUTIONDIR}3rdParty/Poco/)
set(CXX_FLAGS_Release "${CXX_FLAGS} -Wall -O3")
set(CXX_FLAGS_Debug   "${CXX_FLAGS} -Wall -ggdb")
set(FINAL_COMPILE_FLAGS "${CXX_FLAGS_${CMAKE_BUILD_TYPE}}")


include_directories(
	${BOOST} ${POCO}/Foundation/include
	${POCO}/CppUnit/include)


link_directories(
	${BOOST}lib/${PLATFORM}/${ARCH}
    ${POCO}lib/${PLATFORM}/${ARCH})


add_subdirectory(${SOLUTIONDIR}TetrisCore ${OUTDIR}TetrisCore)
add_subdirectory(${SOLUTIONDIR}TetrisCore/testsuite ${OUTDIR}TetrisTestSuite)

