C:\Factory\Petra\PublicLog.exe AutoPwOff é¿çs [%1]

IF "%1" == "" (
	C:\Factory\Labo\Tools\CloseWin.exe /C PipeLogger.exe
) ELSE (
	C:\Factory\Labo\Tools\CloseWin.exe /C "%1"
)

CALL C:\Factory\Tools\PwOff.bat
