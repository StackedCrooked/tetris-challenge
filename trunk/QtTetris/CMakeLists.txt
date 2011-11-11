cmake_minimum_required(VERSION 2.6)
project(QtTetris)
FIND_PACKAGE(Qt4 REQUIRED)


#
# Check TETRIS_BUILD_TYPE environment variable
#
set(CMAKE_BUILD_TYPE $ENV{TETRIS_BUILD_TYPE})
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Release")
message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
else()
set(CMAKE_BUILD_TYPE Release)
endif()


#
# Detect PLATFORM and ARCH variables
#
if(WIN32)
    set(PLATFORM Windows)
    set(ARCH x86)
else()
    execute_process(COMMAND uname OUTPUT_VARIABLE PLATFORM)
    string(REPLACE "\n" "" PLATFORM ${PLATFORM})
    execute_process(COMMAND arch OUTPUT_VARIABLE ARCH)
    string(REPLACE "\n" "" ARCH ${ARCH})
endif()


message("PLATFORM: ${PLATFORM}")
message("ARCH:     ${ARCH}")


#
# Path variables
#
set(EXECUTABLE QtTetris)
set(CMAKE_OSX_ARCHITECTURES ${ARCH})
set(SOLUTIONDIR "${QtTetris_SOURCE_DIR}/../")
set(BUILD_ROOT ${SOLUTIONDIR}Build/)


#
# Set output directory variable
#
set(OUTDIR ${BUILD_ROOT}${CMAKE_BUILD_TYPE}/${PLATFORM}/${ARCH}/)


#
# Now we can set the CMAKE vars
#
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTDIR}lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTDIR}bin/)


#
# Compile flags
#
set(WARNING_OPTIONS_STM "-Wno-strict-aliasing -Wno-deprecated-declarations")
set(WARNING_OPTIONS     "-Wall -Werror ${WARNING_OPTIONS_STM}")
set(CXX_FLAGS         "${CXX_FLAGS} -DTETRIS_QT=1")
set(CXX_FLAGS_Debug   "${CXX_FLAGS} ${WARNING_OPTIONS} -ggdb3")
set(CXX_FLAGS_Release "${CXX_FLAGS} ${WARNING_OPTIONS} -O3 -DNDEBUG")
set(FINAL_COMPILE_FLAGS "${CXX_FLAGS_${CMAKE_BUILD_TYPE}}")


#
# Extra platform specific libs
#
if(WIN32)
    set(EXTRA_LIBS "-lIphlpapi ${EXTRA_LIBS}")
else()
    set(EXTRA_LIBS "pthread")
endif()



#
# Dependencies
#
add_subdirectory(${SOLUTIONDIR}Tetris ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Tetris)
add_subdirectory(${SOLUTIONDIR}Tetris/testing ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Tetris/testing)
add_subdirectory(${SOLUTIONDIR}Futile ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Futile)


#
# Includes
#
include_directories(
    ${SOLUTIONDIR}Tetris/include
    ${SOLUTIONDIR}Futile/include
    ${SOLUTIONDIR}3rdParty/Boost
    ${SOLUTIONDIR}3rdParty/Poco/Foundation/include)


#
# QtTetris source files
#
set(QtTetris_SOURCES
    main.cpp
    MainWindow.cpp
    Model.cpp
    NewGameDialog.cpp
    TetrisWidget.cpp
    QtMainThread.cpp)


#
# QtTetris header files
#
set(QtTetris_QT_HEADERS MainWindow.h TetrisWidget.h NewGameDialog.h)
set(QtTetris_REGULAR_HEADERS MainWindow.h)
set(QtTetris_HEADERS ${QtTetris_QT_HEADERS} ${QtTetris_REGULAR_HEADERS})


#
# Qt resources
#
QT4_ADD_RESOURCES(QtTetris_Resources "QtTetris.qrc")


#
# Special settings to make CMake and Qt work together
#
QT4_WRAP_CPP(QtTetris_HEADERS_MOC ${QtTetris_QT_HEADERS})
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})



#
# Platform specific options for the creation of the executable
#
if(APPLE)
    set(EXECUTABLE_OPTIONS MACOSX_BUNDLE)
else()
    set(EXECUTABLE_OPTIONS "")
endif()


#
# Set the executable. On Mac we want a bundle.
#
add_executable(${EXECUTABLE} ${EXECUTABLE_OPTIONS} ${QtTetris_SOURCES} ${QtTetris_HEADERS_MOC} ${QtTetris_Resources})


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
# APPLE-specific post-build steps:
#  - Make a self-contained (Qt libs included) application bundle using the macdeployqt utility
#  - Copy the Tetris.icns icon to the application Resoures directory
#  - Register this icon in the Info.plist using the PlistBuddy utility.
#
if(APPLE)
set(BUNDLE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}QtTetris.app")
if("$ENV{TETRIS_DEPLOY_DMG}" STREQUAL "Yes")
set(MACDEPLOYQT_OPTIONS "-dmg")
else()
set(MACDEPLOYQT_OPTIONS "")
endif()
add_custom_command(TARGET ${EXECUTABLE}
                   POST_BUILD
                   COMMAND /bin/rm                  ARGS  "-rf" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}QtTetris.app/Contents/Plugins"
                   COMMAND /bin/rm                  ARGS  "-f" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}QtTetris.app/Contents/Resources/qt.conf"
                   COMMAND /bin/mkdir               ARGS  "-p"
                                                          "${BUNDLE}/Contents/Frameworks"
                                                          "${BUNDLE}/Contents/Resources"
                   COMMAND /bin/cp                  ARGS  "${SOLUTIONDIR}Tetris/res/Tetris.icns"
                                                          "${BUNDLE}/Contents/Resources/Tetris.icns"
                   COMMAND /usr/libexec/PlistBuddy  ARGS  "-c" \"Set :CFBundleIconFile Tetris\" "${BUNDLE}/Contents/Info.plist"
                   COMMAND /usr/bin/macdeployqt     ARGS  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}QtTetris.app" "${MACDEPLOYQT_OPTIONS}"
                   WORKING_DIRECTORY  ${BUILD_ROOT})
endif(APPLE)


#
# Libs
#
target_link_libraries(${EXECUTABLE}
    ${QT_LIBRARIES}
    Tetris
    Futile
    ${EXTRA_LIBS}
    ${SOLUTIONDIR}3rdParty/Boost/lib/${PLATFORM}/${ARCH}/libboost_signals.a
    ${SOLUTIONDIR}3rdParty/Boost/lib/${PLATFORM}/${ARCH}/libboost_thread.a
    ${SOLUTIONDIR}3rdParty/Poco/lib/${PLATFORM}/${ARCH}/libPocoFoundation.a)


#
# Target properties
#
set_target_properties(${EXECUTABLE}  PROPERTIES
                      COMPILE_FLAGS  ${FINAL_COMPILE_FLAGS})

