call "%VS90COMNTOOLS%vsvars32.bat"
rmdir /S /Q Build
mkdir Build
cd Build
mkdir QtTetris
cd QtTetris
cmake -G "NMake Makefiles" ..\..\Main\QtTetris

