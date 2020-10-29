:LOOP
CALL %*
C:\Factory\Tools\wait.exe 5
IF NOT ERRORLEVEL 1 GOTO LOOP
