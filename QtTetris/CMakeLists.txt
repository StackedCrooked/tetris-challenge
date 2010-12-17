cmake_minimum_required(VERSION 2.6) 
project(QtTetris)
FIND_PACKAGE(Qt4 REQUIRED)


set(CMAKE_BUILD_TYPE Release)


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
set(SOLUTIONDIR "${QtTetris_SOURCE_DIR}/../")
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



SET(QtTetris_SOURCES main.cpp MainWindow.cpp)
SET(QtTetris_HEADERS MainWindow.h)
QT4_WRAP_CPP(QtTetris_HEADERS_MOC ${QtTetris_HEADERS})
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_EXECUTABLE(QtTetris ${QtTetris_SOURCES} ${QtTetris_HEADERS_MOC})
TARGET_LINK_LIBRARIES(QtTetris ${QT_LIBRARIES})
