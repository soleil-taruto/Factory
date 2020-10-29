rem usage: qrum [(-fÅb--) [-qa]]
if not "%1" == "-f" if /i not "%CD:~0,7%" == "C:\Dev\" goto end

call qq -f
if not "%2" == "-qa" (
rum .
rum /rrr .
) else (
rum /q .
rum /rrra .
)

:end
