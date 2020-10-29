START /B /WAIT /DC:\Factory\Common CL /W2 /WX /Oxt /J /GF /c *.c
DEL C:\Factory\Common\memAllocTest.obj
CL /W2 /WX /Oxt /J /GF _Cx.c libs\*.c C:\Factory\Common\*.obj
DEL *.obj
COPY /B _Cx.exe ..\Cx.exe
