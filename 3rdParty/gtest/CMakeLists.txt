add_library(GTest
    src/gtest-all.cc)

target_include_directories(GTest PUBLIC include PRIVATE "")
