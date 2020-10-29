@set @dirs=C:\Factory C:\Dev C:\pleiades\workspace
fndbrknuuid.exe %@dirs%
IF ERRORLEVEL 1 PAUSE
chkuuidcoll.exe %@dirs%
@set @dirs=
