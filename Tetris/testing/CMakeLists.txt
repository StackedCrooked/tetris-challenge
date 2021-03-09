find_package(GTest)

add_executable(TetrisTest src/main.cpp)

target_link_libraries(TetrisTest PRIVATE Tetris GTest::GTest)
