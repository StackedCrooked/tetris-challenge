rmdir /S /Q Build
mkdir Build
cd Build
mkdir QtTetris
cd QtTetris
cmake -G "MinGW Makefiles" ..\..\Main\QtTetris
mingw32-make VERBOSE=1 -j