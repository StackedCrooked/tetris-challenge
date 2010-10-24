cmake_minimum_required(VERSION 2.6) 
project(TetrisTestSuite) 

set(PLATFORM Darwin)
set(CONFIGURATION Release)
set(ARCH i386)
set(EXECUTABLE tetristestsuite)

set(PROJECT_ROOT ${TetrisTestSuite_SOURCE_DIR}/)
set(BUILD_ROOT ${PROJECT_ROOT}Build/)
set(BUILDDIR ${BUILD_ROOT}${PLATFORM}/${ARCH}/${CONFIGURATION}/)
message("BUILDDIR: ${BUILDDIR}")
set(CMAKEDIR ${BUILD_ROOT}CMake/)
message("CMAKEDIR: ${CMAKEDIR}")

set(BOOST_VERSION 1_44_0)
set(BOOST "Boost_${BOOST_VERSION}")
set(BOOST_ROOT ${PROJECT_ROOT}3rdParty/${BOOST}/)
set(BOOST_LIB_ROOT ${BOOST_ROOT}lib/)
set(BOOST_LIB ${BOOST_LIB_ROOT}${PLATFORM}/${ARCH}/)
message("BOOST_LIB: ${BOOST_LIB}")
message("Source dir: ${TetrisTestSuite_SOURCE_DIR}")


set(POCO_ROOT ${PROJECT_ROOT}3rdParty/Poco/)
set(POCO_LIB_ROOT ${POCO_ROOT}lib/)
set(POCO_LIB ${POCO_LIB_ROOT}${PLATFORM}/${ARCH}/)
set(POCO_FOUNDATION ${POCO_ROOT}Foundation/)
set(POCO_CPPUNIT ${POCO_ROOT}CppUnit/)

include_directories(${BOOST_ROOT})
include_directories(${POCO_FOUNDATION}include)
include_directories(${POCO_CPPUNIT}include)

link_directories(${BOOST_LIB})
message("POCO_LIB: ${POCO_LIB}")
link_directories(${POCO_LIB})

message("CMAKE_PROJECT_NAME: ${CMAKE_PROJECT_NAME}")

add_subdirectory(TetrisCore ${BUILDDIR}TetrisCore)
add_subdirectory("TetrisCore/testsuite" ${BUILDDIR}${CMAKE_PROJECT_NAME})


message("FILE 1:  ${BOOST_LIB}libboost_thread.dylib")
message("FILE 2:  ${POCO_LIB}libPocoFoundation.dylib")
message("FILE 3:  ${POCO_LIB}libCppUnit.dylib")
message("DESTINATION: ${BUILDDIR}${CMAKE_PROJECT_NAME}")

file(COPY			${BOOST_LIB}libboost_thread.dylib
					${POCO_LIB}libPocoFoundation.dylib
			        ${POCO_LIB}libCppUnit.dylib
     DESTINATION    ${BUILDDIR}${CMAKE_PROJECT_NAME}) 
