@ECHO OFF
C:\Factory\Tools\ld2.exe %*
IF EXIST C:\Factory\tmp\LoadDir.bat CALL C:\Factory\tmp\LoadDir.bat
