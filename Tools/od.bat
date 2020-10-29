@ECHO OFF
C:\Factory\Tools\ld2.exe /O %*
IF EXIST C:\Factory\tmp\LoadDir.bat CALL C:\Factory\tmp\LoadDir.bat
