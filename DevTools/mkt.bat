makeTags.exe C:\Factory
IF NOT EXIST C:\Dev\NUL GOTO END
makeTags.exe C:\Dev
>> C:\Dev\tags TYPE C:\Factory\tags
:END
