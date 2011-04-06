cmake_minimum_required(VERSION 2.6)
project(QtTetris)
FIND_PACKAGE(Qt4 REQUIRED)


#
# MinGW
#
if ("x$ENV{TETRIS_MINGW}" STREQUAL "xYes")
message("TETRIS_MINGW is set. Enabling cross-compilation mode.")
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER i586-mingw32msvc-gcc)
set(CMAKE_CXX_COMPILER i586-mingw32msvc-g++)
set(CMAKE_FIND_ROOT_PATH  /usr/i586-mingw32msvc)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()



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
set(BUILD_ROOT ${SOLUTIONDIR}../Build/)


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
set(WARNING_OPTIONS         "-Wall -Wno-sign-compare")
set(WARNING_OPTIONS_Debug   "${WARNING_OPTIONS} -Werror ")
set(CXX_FLAGS         "${CXX_FLAGS} -DTETRIS_QT=1")
set(CXX_FLAGS_Debug   "${CXX_FLAGS} ${WARNING_OPTIONS_Debug} -ggdb3")
set(CXX_FLAGS_Release "${CXX_FLAGS} ${WARNING_OPTIONS_Release} -O3 -DNDEBUG")
set(FINAL_COMPILE_FLAGS "${CXX_FLAGS_${CMAKE_BUILD_TYPE}}")


#
# Platform specific compilar flags
#
if(APPLE)
    set(WARNING_OPTIONS         "-arch ${ARCH} ")
endif()


#
# Poco dependency
#
set(POCO_ROOT ${SOLUTIONDIR}3rdParty/Poco/)

include_directories(
    ${POCO_ROOT}Foundation/include
    ${POCO_ROOT}CppUnit/include)

link_directories(${POCO_ROOT}lib/${PLATFORM}/${ARCH})


#
# Boost dependency
#
set(BOOST ${SOLUTIONDIR}3rdParty/Boost_1_44_0/)
include_directories(${BOOST})
link_directories(${BOOST}lib/${PLATFORM}/${ARCH})
set(LIB_BOOST_THREAD boost_thread)


#
# Tetris & Futile dependencies
#
add_subdirectory(${SOLUTIONDIR}Tetris ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Tetris)
add_subdirectory(${SOLUTIONDIR}Futile ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Futile)
include_directories(
    ${SOLUTIONDIR}Futile/include
    ${SOLUTIONDIR}Tetris/include)


#
# QtTetris source files
#
set(QtTetris_SOURCES
    main.cpp
    MainWindow.cpp
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
add_executable(${EXECUTABLE} ${EXECUTABLE_OPTIONS} ${QtTetris_SOURCES} ${QtTetris_HEADERS_MOC})


#
# Post build step: make a symbolic link to the last build directory
#
add_custom_command(TARGET ${EXECUTABLE}
                   POST_BUILD
                   COMMAND /bin/rm ARGS -f ${BUILD_ROOT}LastBuild
                   COMMAND /bin/ln ARGS -s ${OUTDIR} ${BUILD_ROOT}LastBuild
                   COMMAND /bin/cp ARGS "${SOLUTIONDIR}Tetris/res/avatar/*.gif" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                   WORKING_DIRECTORY    ${BUILD_ROOT})


#
# APPLE-specific post-build steps:
#  - Make a self-contained (Qt libs included) application bundle using the macdeployqt utility
#  - Copy the Tetris.icns icon to the application Resoures directory
#  - Register this icon in the Info.plist using the PlistBuddy utility.
#
if(APPLE)
set(BUNDLE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}QtTetris.app")
if($ENV{TETRIS_DEPLOY_DMG} STREQUAL "Yes")
set(MACDEPLOYQT_OPTIONS "-dmg")
else()
set(MACDEPLOYQT_OPTIONS "")
endif()
add_custom_command(TARGET ${EXECUTABLE}
                   POST_BUILD
                   COMMAND /bin/rm                  ARGS  "-rf" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}QtTetris.app/Contents/Plugins"
                   COMMAND /bin/rm                  ARGS  "-f" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}QtTetris.app/Contents/Resources/qt.conf"
                   COMMAND /bin/mkdir               ARGS  "-p" "${BUNDLE}/Contents/Resources"
                   COMMAND /bin/cp                  ARGS  "${SOLUTIONDIR}Tetris/res/Tetris.icns" "${BUNDLE}/Contents/Resources/Tetris.icns"
                   COMMAND /usr/libexec/PlistBuddy  ARGS  "-c" \"Set :CFBundleIconFile Tetris\" "${BUNDLE}/Contents/Info.plist"
                   COMMAND /usr/bin/macdeployqt     ARGS  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}QtTetris.app" "${MACDEPLOYQT_OPTIONS}"
                   WORKING_DIRECTORY  ${BUILD_ROOT})
endif(APPLE)


#
# QtTetris libs
#
target_link_libraries(${EXECUTABLE}
    ${QT_LIBRARIES}
    Tetris
    Futile
    PocoFoundation
    ${LIB_BOOST_THREAD})


#
# Target properties
#
set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS ${FINAL_COMPILE_FLAGS})

