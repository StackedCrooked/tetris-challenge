@echo off
call clean.bat
call configure.bat
cd Build
jom
cd ..