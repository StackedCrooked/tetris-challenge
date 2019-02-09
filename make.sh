#!/bin/bash
set -e

[ ! -d Build ] && ./configure.sh

cd Build
make -j7

# Note
# ----
# The produced binary needs to know where to find the Qt libs. We tell it to
# look for the frameworks in the Qt installation directory where we found "moc".
{ otool -L bin/QtTetris | grep 'rpath'; } >/dev/null || {
    QT_LIB_DIR="$(dirname "$(dirname $(which moc))")/lib"
    install_name_tool -add_rpath $QT_LIB_DIR bin/QtTetris
}


# Run the unit tests 
./bin/TetrisTest

