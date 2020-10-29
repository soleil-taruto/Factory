DEL C:\temp\hkcu_shh_reg.tmp
DEL C:\temp\hkcu_shh_txt.tmp
reg export HKCU\Software\Hidemaruo\Hidemaru_xxx C:\temp\hkcu_shh_reg.tmp
RegExpToText.exe C:\temp\hkcu_shh_reg.tmp       C:\temp\hkcu_shh_txt.tmp
ShowSetting.exe  C:\temp\hkcu_shh_txt.tmp
