rem usage: addmod mm MODULE-NODE

IF "%2" == "" GOTO END
IF EXIST %2.cpp GOTO END
IF EXIST %2.h GOTO END
IF NOT EXIST all.h GOTO END
GOTO _%1

:_MM
> %2.cpp ECHO #include "all.h"
> %2.h ECHO;
>> all.h ECHO #include "%2.h"
GOTO END

:END
