#!/bin/bash
set -e

[ ! -d Build ] && {
    ./configure.sh
}

cd Build
make -j7

# Run the unit tests 
Tetris/testing/TetrisTest
