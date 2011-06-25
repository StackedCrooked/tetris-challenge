#!/bin/sh
ASTYLE_ARGS="--indent=spaces=4 --brackets=break --indent-switches --indent-labels --pad-header --unpad-paren --convert-tabs --align-pointer=middle --suffix=none --lineend=linux"

for File in $* ; do
    astyle $ASTYLE_ARGS $File
done
