START /MIN CMD /C FOR /L %%C IN (1, 1, 999) DO RD /S /Q C:\%%C
DEL C:\Factory\tmp_data\zzEz.txt
