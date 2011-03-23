cmake_minimum_required(VERSION 2.6)
project(MinGWTetris)

set(CMAKE_BUILD_TYPE Release)

set(SOLUTIONDIR ${MinGWTetris_SOURCE_DIR}/../../)
set(BUILD_ROOT  ${SOLUTIONDIR}../Build/)
set(OUTDIR      ${BUILD_ROOT}${CMAKE_BUILD_TYPE}/)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTDIR}lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTDIR}bin/)

set(POCO  ${SOLUTIONDIR}3rdParty/Poco/)
set(BOOST ${SOLUTIONDIR}3rdParty/Boost_1_44_0/)

include_directories(
    ${BOOST}
    ${POCO}Foundation/include
    ${POCO}CppUnit/include
    ${SOLUTIONDIR}Futile/include
    ${SOLUTIONDIR}Tetris/include
    ${SOLUTIONDIR}QtTetris/include)

link_directories(
    ${BOOST}lib/Windows/i686
    ${POCO}lib/Windows/i686)  # We have now MinGW builds ready for Poco

add_subdirectory(${SOLUTIONDIR}Tetris ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Tetris)
add_subdirectory(${SOLUTIONDIR}Futile ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Futile)

add_executable(MinGWTetris main.cpp)
target_link_libraries(MinGWTetris
    ${QT_LIBRARIES}
    boost_thread
    PocoFoundation
    Futile
    Tetris
    QtTetris)


