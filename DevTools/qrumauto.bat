rem usage: qrumauto COMMENT
if "%1" == "" goto end

rum /c %1

call qq -f
rum /q .

rum /c-

:end
