cmake_minimum_required(VERSION 2.6)
project(QtTetris)
message("TETRIS_BUILD_CONFIGURATION: $ENV{TETRIS_BUILD_CONFIGURATION}")
set(CMAKE_BUILD_TYPE $ENV{TETRIS_BUILD_CONFIGURATION})
FIND_PACKAGE(Qt4 REQUIRED)


if(WIN32)
    set(ARCH x86)
    set(PLATFORM Windows)
else()
    execute_process(COMMAND uname OUTPUT_VARIABLE PLATFORM)
    string(REPLACE "\n" "" PLATFORM ${PLATFORM})
    execute_process(COMMAND arch OUTPUT_VARIABLE ARCH)
    string(REPLACE "\n" "" ARCH ${ARCH})
endif()

set(EXECUTABLE QtTetris)
set(CMAKE_OSX_ARCHITECTURES ${ARCH})
set(SOLUTIONDIR "${QtTetris_SOURCE_DIR}/../")
set(OUTDIR ${SOLUTIONDIR}Build/${PLATFORM}/${ARCH}/${CMAKE_BUILD_TYPE}/)
set(POCO ${SOLUTIONDIR}3rdParty/Poco/)
set(WARNING_OPTIONS "-arch ${ARCH} -Wall -Werror -Wno-sign-compare")
set(CXX_FLAGS         "${CXX_FLAGS} -DTETRIS_QT=1")
set(CXX_FLAGS_Release "${CXX_FLAGS} ${WARNING_OPTIONS} -O3 -DNDEBUG")
set(CXX_FLAGS_Debug   "${CXX_FLAGS} ${WARNING_OPTIONS} -ggdb3")
set(FINAL_COMPILE_FLAGS "${CXX_FLAGS_${CMAKE_BUILD_TYPE}}")


# Poco dependency
include_directories(${POCO}Foundation/include ${POCO}CppSquare/include ${SOLUTIONDIR}Tetris/include)
link_directories(${POCO}lib/${PLATFORM}/${ARCH})

# Boost dependency
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


add_subdirectory(${SOLUTIONDIR}Tetris ${OUTDIR}Tetris)



set(QtTetris_SOURCES
    main.cpp
    MainWindow.cpp
    TetrisWidget.cpp
    QtMainThread.cpp)

set(QtTetris_HEADERS
    MainWindow.h
    TetrisWidget.h
    QtMainThread.h)

# Qt settings
QT4_WRAP_CPP(QtTetris_HEADERS_MOC ${QtTetris_HEADERS})
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})


add_executable(${EXECUTABLE} ${QtTetris_SOURCES} ${QtTetris_HEADERS_MOC})
set(CONFIG  "${CONFIG} i386")
target_link_libraries(${EXECUTABLE}
    ${QT_LIBRARIES}
    Tetris
    PocoFoundation
    ${LIB_BOOST_THREAD})

set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS ${FINAL_COMPILE_FLAGS})
