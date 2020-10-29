rem usage: qrum [-qa]

start /B /WAIT /DC:\Factory cmd /c Clean.bat

if not "%1" == "-qa" (
rum /f C:\Factory
rum /rrr C:\Factory
) else (
rum /f /q C:\Factory
rum /rrra C:\Factory
)
