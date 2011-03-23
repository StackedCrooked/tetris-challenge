FIND_PACKAGE(Qt4 REQUIRED)



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
add_library(QtTetris ${EXECUTABLE_OPTIONS} ${QtTetris_SOURCES} ${QtTetris_HEADERS_MOC})

set_target_properties(QtTetris PROPERTIES COMPILE_FLAGS ${FINAL_COMPILE_FLAGS})

