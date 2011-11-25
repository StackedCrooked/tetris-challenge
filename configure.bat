@echo off
call %VS100COMNTOOLS%\vsvars32.bat
mkdir Build
cd Build
cmake -G "NMake Makefiles JOM" ..\QtTetris
echo Type "cd Build" followed by "jom" to start building.
cd ..
