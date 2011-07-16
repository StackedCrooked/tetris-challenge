#!/bin/sh
PLATFORM=`uname -o`
ARCH=`arch`
URL="https://tetris-challenge.googlecode.com/svn/attic/Poco_1_3_6_p2/lib/$PLATFORM/$ARCH"
mkdir -p $PLATFORM
cd $PLATFORM
echo "Checking out libs from $URL"
svn co $URL
