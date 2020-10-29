rem usage:
rem new m MODULE-NODE (’P“Æ)
rem new h HEADER-NODE (’P“Æ)
rem new mm MODULE-NODE (all.h)
rem new hh HEADER-NODE (all.h)
rem new e EXECUTABLE-NODE
rem end usage

IF "%2" == "" GOTO END
IF EXIST %2.c GOTO END
rem IF EXIST %2.cpp GOTO END
IF EXIST %2.h GOTO END
GOTO _%1

:_M
> %2.c ECHO #include "%2.h"
:_H
> %2.h ECHO #pragma once
>> %2.h ECHO;
>> %2.h ECHO #include "C:\Factory\Common\all.h"
GOTO END

:_MM
> %2.c ECHO #include "all.h"
:_HH
> %2.h ECHO;
IF EXIST all.h GOTO EALLH
> all.h ECHO #pragma once
>> all.h ECHO;
:EALLH
>> all.h ECHO #include "%2.h"
GOTO END

:_E
> %2.c ECHO #include "C:\Factory\Common\all.h"
>> %2.c ECHO;
>> %2.c ECHO int main(int argc, char **argv)
>> %2.c ECHO {
>> %2.c ECHO }

:END
