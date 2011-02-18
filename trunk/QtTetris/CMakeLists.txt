cmake_minimum_required(VERSION 2.6)
project(QtTetris)
message("TETRIS_BUILD_CONFIGURATION: $ENV{TETRIS_BUILD_CONFIGURATION}")
set(CMAKE_BUILD_TYPE $ENV{TETRIS_BUILD_CONFIGURATION})
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
# Path variables
#
set(EXECUTABLE QtTetris)
set(CMAKE_OSX_ARCHITECTURES ${ARCH})
set(SOLUTIONDIR "${QtTetris_SOURCE_DIR}/../")
set(BUILD_ROOT ${SOLUTIONDIR}Build/)
set(POCO ${SOLUTIONDIR}3rdParty/Poco/)


#
# Set output directory variable
#
# Note:
# - OUTDIR is a user-variable
# - use the second definition for more flexibility.
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
set(WARNING_OPTIONS         "-arch ${ARCH} ")
set(WARNING_OPTIONS_Debug   "${WARNING_OPTIONS} -Wall -Werror -Wno-sign-compare")
set(CXX_FLAGS         "${CXX_FLAGS} -DTETRIS_QT=1")
set(CXX_FLAGS_Debug   "${CXX_FLAGS} ${WARNING_OPTIONS_Debug} -ggdb3")
set(CXX_FLAGS_Release "${CXX_FLAGS} ${WARNING_OPTIONS_Release} -O3 -DNDEBUG")
set(FINAL_COMPILE_FLAGS "${CXX_FLAGS_${CMAKE_BUILD_TYPE}}")


#
# Poco dependency
#
include_directories(${POCO}Foundation/include ${POCO}CppSquare/include ${SOLUTIONDIR}Tetris/include)
link_directories(${POCO}lib/${PLATFORM}/${ARCH})


#
# Boost dependency
#
set(BOOST ${SOLUTIONDIR}3rdParty/Boost_1_44_0/)
include_directories(${BOOST})
link_directories(${BOOST}lib/${PLATFORM}/${ARCH})
set(LIB_BOOST_THREAD boost_thread)


#
# Tetris dependency
#
add_subdirectory(${SOLUTIONDIR}Tetris ${LIBRARY_OUTPUT_PATH})


#
# QtTetris source files
#
set(QtTetris_SOURCES
    main.cpp
    MainWindow.cpp
    TetrisWidget.cpp
    QtMainThread.cpp)


#
# QtTetris header files
#
set(QtTetris_QT_HEADERS MainWindow.h TetrisWidget.h)
set(QtTetris_REGULAR_HEADERS MainWindow.h)
set(QtTetris_HEADERS ${QtTetris_QT_HEADERS} ${QtTetris_REGULAR_HEADERS})


#
# Special settings to make CMake and Qt work together
#
QT4_WRAP_CPP(QtTetris_HEADERS_MOC ${QtTetris_QT_HEADERS})
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})


#
# Set the executable
#
add_executable(${EXECUTABLE} ${QtTetris_SOURCES} ${QtTetris_HEADERS_MOC})

add_custom_command(TARGET               ${EXECUTABLE}
                   POST_BUILD
                   COMMAND              /bin/ln
                   ARGS                 -fs ${OUTDIR} ${BUILD_ROOT}LastBuild
                   WORKING_DIRECTORY    ${BUILD_ROOT})


#
# QtTetris libs
#
target_link_libraries(${EXECUTABLE}
    ${QT_LIBRARIES}
    Tetris
    PocoFoundation
    ${LIB_BOOST_THREAD})


#
# Target properties
#
set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS ${FINAL_COMPILE_FLAGS})
