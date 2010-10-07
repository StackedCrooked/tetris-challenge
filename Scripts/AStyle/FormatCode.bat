@echo off
if "%1" == "" goto error

SET ASTYLE_COMMAND=Scripts\AStyle\AStyle.exe --indent=spaces=4 --brackets=break --indent-switches --indent-namespaces --indent-labels --indent-preprocessor --pad-header --unpad-paren --convert-tabs --align-pointer=middle --suffix=none --lineend=windows --keep-one-line-blocks --keep-one-line-statements --recursive
%ASTYLE_COMMAND% %1\*.h
%ASTYLE_COMMAND% %1\*.cpp
goto :end

:error
echo Usage: Scripts\AStyle\FormatCode.bat PATH

:end