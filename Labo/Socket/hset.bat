rem usage: hset URL LOCAL-FILE [PASSWORD [PROXY-DOMAIN PROXY-PORT]]
IF "%2" == "" GOTO END
FOR /F "DELIMS=" %%A IN ('C:\Factory\DevTools\uuid.exe /P') DO SET X_HSET_UUID=%%A
IF "%3" == "" (
	%~dp0mmpc.exe /S delete %~nx2 %X_HSET_UUID%_content.tmp
	%~dp0mmpc.exe /S delete %~nx2.clu %X_HSET_UUID%_content2.tmp
) ELSE (
	%~dp0mmpc.exe /S delete %~nx2 /S password %3 %X_HSET_UUID%_content.tmp
	%~dp0mmpc.exe /S delete %~nx2.clu /S password %3 %X_HSET_UUID%_content2.tmp
)
IF "%5" == "" (
	%~dp0hget.exe /C %X_HSET_UUID%_content.tmp /O * %1
	%~dp0hget.exe /C %X_HSET_UUID%_content2.tmp /O * %1
) ELSE (
	%~dp0hget.exe /PS %4 /PP %5 /C %X_HSET_UUID%_content.tmp /O * %1
	%~dp0hget.exe /PS %4 /PP %5 /C %X_HSET_UUID%_content2.tmp /O * %1
)
DEL %X_HSET_UUID%_content.tmp
DEL %X_HSET_UUID%_content2.tmp
SET X_HSET_UUID=
CALL %~dp0hput.bat %*
:END
