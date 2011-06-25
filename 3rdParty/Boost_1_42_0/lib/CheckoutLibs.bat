echo off
if exist "Windows\x86" goto already_exists
set URL="https://tetris-challenge.googlecode.com/svn/attic/Boost_1_42_0/lib/Windows/x86"
mkdir Windows\x86
cd "Windows"
echo Downloading Boost libs.
svn co %URL%
goto end

:already_exists
echo Boost libs already downloaded. Ok.

:end
