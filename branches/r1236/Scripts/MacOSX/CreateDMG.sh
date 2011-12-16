#!/bin/sh

# Set environment variables for CMake
export TETRIS_BUILD_TYPE=Release
export TETRIS_DEPLOY_DMG=No

PLATFORM=`uname`
if [ "$PLATFORM" = 'Darwin' ]; then
    export TETRIS_DEPLOY_DMG=Yes
fi

echo "TETRIS_BUILD_TYPE: $TETRIS_BUILD_TYPE"
echo "TETRIS_DEPLOY_DMG: $TETRIS_DEPLOY_DMG"


# First clean the previous builds
rm -rf Build
mkdir -p Build
cd Build
cmake ../QtTetris
make -w -j8

