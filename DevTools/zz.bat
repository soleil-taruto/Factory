> C:\Factory\tmp\zzBefore.bat C:\Factory\DevTools\seq.exe "@REN C:\\" /C 1 1 30 1 " " /C 1 901 99 1
CALL C:\Factory\tmp\zzBefore.bat 2> NUL
	START      CMD /C FOR /L %%C IN (1, 1, 999) DO RD /S /Q C:\%%C
rem START /MIN CMD /C FOR /L %%C IN (1, 1, 999) DO RD /S /Q C:\%%C
