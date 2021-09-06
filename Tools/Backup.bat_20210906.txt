C:\Factory\Tools\RDMD.exe /RM C:\888
C:\Factory\Tools\Backup_Main.exe | C:\Factory\Tools\PipeLogger.exe C:\888\Backup.log
COPY /Y C:\888\Backup.log C:\vaz\Backup.log
IF EXIST C:\888\Extra_0001.bat CALL C:\888\Extra_0001.bat
