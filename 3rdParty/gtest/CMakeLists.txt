include_directories(
    ${CMAKE_SOURCE_DIR}/3rdParty/gtest/include
    ${CMAKE_SOURCE_DIR}/3rdParty/gtest
)

add_library(gtest
    src/gtest-all.cc)
