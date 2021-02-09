rem シャットダウンします。
C:\Factory\Elsa\PublicLog.exe PwOff 実行
C:\Factory\Tools\wait.exe 30
IF ERRORLEVEL 1 GOTO END
shutdown /s /t 30
rem PAUSE
:END
