C:\Factory\Petra\PublicLog.exe AutoPwOff 対象プロセス = [%1]

IF "%1" == "" (
	C:\Factory\Labo\Tools\CloseWin.exe /C PipeLogger.exe
) ELSE (
	C:\Factory\Labo\Tools\CloseWin.exe /C "%1"
)

C:\Factory\Petra\PublicLog.exe AutoPwOff 対象プロセスの終了を確認した。

CALL C:\Factory\Tools\PwOff.bat
