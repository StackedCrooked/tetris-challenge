echo off
if exist "Windows\x86" goto already_exists
set URL="https://tetris-challenge.googlecode.com/svn/attic/Poco_1_3_6_p2/lib/Windows/x86"
mkdir Windows
cd "Windows"
echo Downloading Poco libs.
svn co %URL% x86
goto end

:already_exists
echo Poco libs already downloaded. Ok.

:end
