CALL ff
CALL Clean
CALL mkt
RDMD.exe /RM tmp_Prime

REM =============
REM == .cmp.gz ==
REM =============

RDMD.exe /RM C:\999
C:\apps\Compress\Compress.exe . C:\999\Factory.cmp.gz
START C:\999

REM ==========
REM == .zip ==
REM ==========

z7 .

REM ====
REM ====
REM ====

CALL dittybox\oninstall.bat
