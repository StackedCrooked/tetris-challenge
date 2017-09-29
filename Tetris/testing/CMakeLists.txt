include_directories(
    ${CMAKE_SOURCE_DIR}/3rdParty/gtest/include
    ${CMAKE_SOURCE_DIR}/Futile/include
    ${CMAKE_SOURCE_DIR}/Tetris/include)


add_executable(TetrisTest
    src/main.cpp)


target_link_libraries(TetrisTest
    gtest)
