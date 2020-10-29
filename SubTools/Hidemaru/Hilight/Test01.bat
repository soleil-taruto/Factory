DEL C:\temp\1.tmp
DEL C:\temp\2.tmp
DEL C:\temp\3.tmp
DEL C:\temp\4.tmp
reg export HKCU\Software\Hidemaruo\Hidemaru C:\temp\1.tmp
..\RegExpToText.exe C:\temp\1.tmp   C:\temp\2.tmp
..\ShowSetting.exe  C:\temp\2.tmp > C:\temp\3.tmp
MkHilight.exe C:\temp\3.tmp 1.txt
