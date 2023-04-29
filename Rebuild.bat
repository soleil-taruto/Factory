CALL ff
CD Build\_Cx
CALL Rebuild.bat

CALL ff
cx ***
IF ERRORLEVEL 1 C:\app\MsgBox\MsgBox.exe E "BUILD ERROR"

CALL bat\oninstall.bat
