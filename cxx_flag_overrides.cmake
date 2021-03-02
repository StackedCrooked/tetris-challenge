# Here we choose custom flags for the Release build type (CMAKE_BUILD_TYPE=Release):
# - optimization level is -O2
# - enable debug info
# - leave assertions enabled (don't define "-DNDEBUG")
# - optimize for the architecture of the build machine
set(CMAKE_CXX_FLAGS_RELEASE_INIT " -O2 -g -march=native ")
