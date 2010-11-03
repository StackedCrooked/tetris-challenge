cmake_minimum_required(VERSION 2.6) 
project(TetrisTestSuite) 

set(CONFIGURATION Release)

if(APPLE)
set(ARCH "i386")
set(PLATFORM Darwin)
else()
set(PLATFORM Linux)
endif()

set(EXECUTABLE tetristestsuite)

set(PROJECT_ROOT ${TetrisTestSuite_SOURCE_DIR}/)
set(BUILD_ROOT ${PROJECT_ROOT}Build/)

if(APPLE)
set(BUILDDIR ${BUILD_ROOT}${PLATFORM}/${ARCH}/${CONFIGURATION}/)
else()
set(BUILDDIR ${BUILD_ROOT}${PLATFORM}/${CONFIGURATION}/)
endif()

set(CMAKEDIR ${BUILD_ROOT}CMake/)

set(BOOST_VERSION 1_44_0)
set(BOOST "Boost_${BOOST_VERSION}")
set(BOOST_ROOT ${PROJECT_ROOT}3rdParty/${BOOST}/)
set(BOOST_LIB_ROOT ${BOOST_ROOT}lib/)

if(APPLE)
set(BOOST_LIB ${BOOST_LIB_ROOT}${PLATFORM}/${ARCH}/)
else()
set(BOOST_LIB ${BOOST_LIB_ROOT}${PLATFORM}/)
endif()


set(POCO_ROOT ${PROJECT_ROOT}3rdParty/Poco/)
set(POCO_LIB_ROOT ${POCO_ROOT}lib/)
set(POCO_LIB ${POCO_LIB_ROOT}${PLATFORM}/${ARCH}/)
set(POCO_FOUNDATION ${POCO_ROOT}Foundation/)
set(POCO_CPPUNIT ${POCO_ROOT}CppUnit/)

#include_directories(${BOOST_ROOT})
include_directories("/usr/local/include")
include_directories(${POCO_FOUNDATION}include)
include_directories(${POCO_CPPUNIT}include)

#link_directories(${BOOST_LIB})
link_directories("/usr/local/lib")
link_directories(${POCO_LIB})

# FIXME: this is a temporary fix
link_directories("/home/francis/orig/poco-1.3.6p2/lib/Linux/x86_64")


add_subdirectory(TetrisCore ${BUILDDIR}TetrisCore)
add_subdirectory("TetrisCore/testsuite" ${BUILDDIR}${CMAKE_PROJECT_NAME})


if(APPLE)
file(COPY			${BOOST_LIB}libboost_thread.dylib
					${POCO_LIB}libPocoFoundation.dylib
			        ${POCO_LIB}libCppUnit.dylib
     DESTINATION    ${BUILDDIR}${CMAKE_PROJECT_NAME}) 
endif(APPLE)

