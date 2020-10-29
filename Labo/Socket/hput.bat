rem usage: hput URL LOCAL-FILE [PASSWORD [PROXY-DOMAIN PROXY-PORT]]
rem usage: hput URL * [PASSWORD [PROXY-DOMAIN PROXY-PORT]]
IF "%2" == "" GOTO END
FOR /F "DELIMS=" %%A IN ('C:\Factory\DevTools\uuid.exe /P') DO SET X_HPUT_UUID=%%A
IF "%3" == "" (
	%~dp0mmpc.exe /S upload upload /F file %2 %X_HPUT_UUID%_content.tmp
) ELSE (
	%~dp0mmpc.exe /S upload upload /S password %3 /F file %2 %X_HPUT_UUID%_content.tmp
)
IF NOT EXIST %X_HPUT_UUID%_content.tmp GOTO END
IF "%5" == "" (
	%~dp0hget.exe /BT 3600 /C %X_HPUT_UUID%_content.tmp /O * %1
) ELSE (
	%~dp0hget.exe /BT 3600 /PS %4 /PP %5 /C %X_HPUT_UUID%_content.tmp /O * %1
)
DEL %X_HPUT_UUID%_content.tmp
SET X_HPUT_UUID=
:END
