#!/bin/sh
PLATFORM=`uname`
ARCH=`arch`
URL="https://tetris-challenge.googlecode.com/svn/attic/Boost_1_42_0/lib/$PLATFORM/$ARCH"
mkdir -p $PLATFORM
cd $PLATFORM
echo "Checking out libs from $URL"
svn co $URL
