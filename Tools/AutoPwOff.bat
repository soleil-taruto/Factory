C:\Factory\Petra\PublicLog.exe AutoPwOff �Ώۃv���Z�X = [%1]

IF "%1" == "" (
	C:\Factory\Labo\Tools\CloseWin.exe /C PipeLogger.exe
) ELSE (
	C:\Factory\Labo\Tools\CloseWin.exe /C "%1"
)

C:\Factory\Petra\PublicLog.exe AutoPwOff �Ώۃv���Z�X�̏I�����m�F�����B

CALL C:\Factory\Tools\PwOff.bat
