cmake_minimum_required(VERSION 2.8)
project(QtTetris)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets)
find_package(Qt5 COMPONENTS Core Gui Widgets)

#
# Check TETRIS_BUILD_TYPE environment variable
#
set(ENV_BUILD_TYPE $ENV{TETRIS_BUILD_TYPE})
message("ENV_BUILD_TYPE is ${ENV_BUILD_TYPE}")
if(ENV_BUILD_TYPE STREQUAL "Debug" OR ENV_BUILD_TYPE STREQUAL "Release" OR ENV_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CMAKE_BUILD_TYPE ${ENV_BUILD_TYPE})
else()
    set(CMAKE_BUILD_TYPE Release)
endif()
message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")


#
# Detect PLATFORM and ARCH variables
#
if(WIN32)
    set(PLATFORM Windows)
    set(ARCH x86)
elseif(APPLE)
    set(PLATFORM Darwin)
    set(ARCH x86_64)
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
set(SOLUTIONDIR "${QtTetris_SOURCE_DIR}/../")


#
# Set output directory variable
#
set(OUTDIR ${CMAKE_BUILD_TYPE})

set (CMAKE_CXX_FLAGS "-std=c++11 -g3 -Wall -O0")
#
# Compile flags
#
if(WIN32)
    set(CXX_FLAGS_Debug   "${CXX_FLAGS} /DTETRIS_QT=1 ")
    set(CXX_FLAGS_Release "${CXX_FLAGS} /DTETRIS_QT=0 ")
    set(FINAL_COMPILE_FLAGS "${CXX_FLAGS_${CMAKE_BUILD_TYPE}} /D FUTILE_EVENTLOOP /D POCO_STATIC /D POCO_NO_AUTOMATIC_LIBS /D BOOST_ALL_NO_LIB /D _HAS_ITERATOR_DEBUGGING=0 /D _SECURE_SCL=0 /D _SECURE_SCL_THROWS=0")
else()
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")
    set(CXX_FLAGS         "${CXX_FLAGS} -std=c++11 -DTETRIS_QT=1 ")
#    if(${CMAKE_CXX_COMPILER} MATCHES ".*clang.*")
#        set(CXX_FLAGS     "${CXX_FLAGS} -stdlib=libc++ -Wno-self-assign -Wno-reserved-user-defined-literal -Qunused-arguments ")
#    endif()
    set(WARNING_OPTIONS_STM " -Wall -Wextra -Werror ")
    set(WARNING_OPTIONS          " ${WARNING_OPTIONS_STM} ")
    set(CXX_FLAGS_Debug          " ${WARNING_OPTIONS} ${CXX_FLAGS} -ggdb3 ")
    set(CXX_FLAGS_Release        " ${WARNING_OPTIONS} ${CXX_FLAGS} -O3 -DNDEBUG ")
    set(CXX_FLAGS_RelWithDebInfo " ${WARNING_OPTIONS} ${CXX_FLAGS} -O1 -g -DNDEBUG ")
    set(FINAL_COMPILE_FLAGS "${CXX_FLAGS_${CMAKE_BUILD_TYPE}} -DFUTILE_EVENTLOOP ")
endif()


#
# Extra platform specific libs
#
if(WIN32)
    set(EXTRA_LIBS "Iphlpapi.lib" "${EXTRA_LIBS}")
else()
    set(EXTRA_LIBS "pthread")
endif()


#
# Dependencies
#
add_subdirectory(${SOLUTIONDIR}Tetris ${OUTDIR}/Tetris)
add_subdirectory(${SOLUTIONDIR}Futile ${OUTDIR}/Futile)


#
# Includes
#
include_directories(SYSTEM
    ${QtSDK}/include
    ${SOLUTIONDIR}3rdParty/stm
    /usr/include)

set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")

include_directories(
    ${SOLUTIONDIR}Tetris/include
    ${SOLUTIONDIR}Futile/include)


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

QT5_ADD_RESOURCES(QtTetris_Resources "QtTetris.qrc")


#
# Special settings to make CMake and Qt work together
#
QT5_WRAP_CPP(QtTetris_HEADERS_MOC ${QtTetris_QT_HEADERS})
#include(${QT_USE_FILE})
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
add_executable(QtTetris   ${EXECUTABLE_OPTIONS} ${QtTetris_SOURCES} ${QtTetris_HEADERS_MOC} ${QtTetris_Resources})
qt5_use_modules(QtTetris Widgets)


#
# Compiler flags
#
set_target_properties(QtTetris  PROPERTIES COMPILE_FLAGS  ${FINAL_COMPILE_FLAGS})



#
# Libraries
#
set(LIBEXT "a")
if(WIN32)
set(LIBEXT "lib")
endif()


#
# Clang requires explicty dylib.
#
if(${CMAKE_CXX_COMPILER} MATCHES ".*clang.*")
set(CLANG_LIBCPP "/usr/local/lib/libc++.1.dylib")
else()
set(CLANG_LIBCPP "")
endif()


target_link_libraries(QtTetris -lc++ ${QT_LIBRARIES} Tetris Futile ${EXTRA_LIBS} -L/usr/lib -lPocoFoundation /usr/lib/x86_64-linux-gnu/libboost_thread.a /usr/lib/x86_64-linux-gnu/libboost_system.a) 

#
# Post build step: make a symbolic link to the last build directory
#
if(WIN32)
# TODO: Add unix shell support. Or use CMake builtin commands.
else()
add_custom_command(TARGET QtTetris
                   POST_BUILD
                   COMMAND /bin/rm ARGS -f LastBuild
                   COMMAND /bin/ln ARGS -s ${OUTDIR} LastBuild
                   WORKING_DIRECTORY    )
endif()

#
# APPLE-specific post-build steps:
#  - Make a self-contained (Qt libs included) application bundle using the macdeployqt utility
#  - Copy the Tetris.icns icon to the application Resoures directory
#  - Register this icon in the Info.plist using the PlistBuddy utility.
#
if(APPLE)
set(BUNDLE "${OUTDIR}/QtTetris/QtTetris.app")
if("$ENV{TETRIS_DEPLOY_DMG}" STREQUAL "Yes")
set(MACDEPLOYQT_OPTIONS "-dmg")
else()
set(MACDEPLOYQT_OPTIONS "")
endif()
add_custom_command(TARGET QtTetris
                   POST_BUILD
                   COMMAND /bin/rm                  ARGS  "-rf" "${BUNDLE}"
                   COMMAND /bin/mkdir               ARGS  "-p"
                                                          "${BUNDLE}/Contents/Frameworks"
                                                          "${BUNDLE}/Contents/Resources"
                   COMMAND /bin/cp                  ARGS  "${SOLUTIONDIR}Tetris/res/Tetris.icns" "${BUNDLE}/Contents/Resources/Tetris.icns"
                   COMMAND /usr/libexec/PlistBuddy  ARGS  "-c" \"Add :CFBundleIconFile string\" "${BUNDLE}/Contents/Info.plist"
                   COMMAND /usr/libexec/PlistBuddy  ARGS  "-c" \"Set :CFBundleIconFile Tetris\" "${BUNDLE}/Contents/Info.plist"
                   WORKING_DIRECTORY  )
endif(APPLE)

