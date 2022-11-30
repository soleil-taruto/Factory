IF "%FactorySetEnvDone%" == "1" GOTO END

rem 2022
CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
rem 2019
rem CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
rem 2010
rem CALL "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"

ECHO ON

PATH=%PATH%;C:\Factory\Build
PATH=%PATH%;C:\Factory\DevTools
PATH=%PATH%;C:\Factory\Tools

SET FactorySetEnvDone=1
:END
