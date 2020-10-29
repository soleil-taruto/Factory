@set @dirs=C:\Factory C:\Dev
fndbrknuuid.exe %@dirs%
IF ERRORLEVEL 1 PAUSE
chkuuidcoll.exe %@dirs%
@set @dirs=
