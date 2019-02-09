#!/bin/bash
set -e

mkdir -p Build && cd Build

# Install the conan dependencies.
MAKEFLAGS=${MAKEFLAGS:--j7} conan install .. --build=missing 
cmake .. -DCMAKE_CXX_COMPILER_LAUNCHER=ccache 
