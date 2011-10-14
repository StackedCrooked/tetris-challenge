#!/bin/sh
#LD_LIBRARY_PATH=./QtTetris
APPDIR="`dirname \"$0\"`"
echo "$APPDIR"
LD_LIBRARY_PATH=$APPDIR:$LD_LIBRARY_PATH $APPDIR/QtTetris


