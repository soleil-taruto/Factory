rem gen-src

C:\Factory\SubTools\rept.exe /P $BIT 64 /P $hBIT 32 /P $SZ 2 /P $hSZ 1 uintx.c.h ..\uint64.c
C:\Factory\SubTools\rept.exe /P $BIT 64 /P $hBIT 32 /P $SZ 2 /P $hSZ 1 uintx.h.h ..\uint64.h

C:\Factory\SubTools\rept.exe /P $BIT 128 /P $hBIT 64 /P $SZ 4 /P $hSZ 2 uintx.c.h ..\uint128.c
C:\Factory\SubTools\rept.exe /P $BIT 128 /P $hBIT 64 /P $SZ 4 /P $hSZ 2 uintx.h.h ..\uint128.h

C:\Factory\SubTools\rept.exe /P $BIT 256 /P $hBIT 128 /P $SZ 8 /P $hSZ 4 uintx.c.h ..\uint256.c
C:\Factory\SubTools\rept.exe /P $BIT 256 /P $hBIT 128 /P $SZ 8 /P $hSZ 4 uintx.h.h ..\uint256.h

C:\Factory\SubTools\rept.exe /P $BIT 512 /P $hBIT 256 /P $SZ 16 /P $hSZ 8 uintx.c.h ..\uint512.c
C:\Factory\SubTools\rept.exe /P $BIT 512 /P $hBIT 256 /P $SZ 16 /P $hSZ 8 uintx.h.h ..\uint512.h

C:\Factory\SubTools\rept.exe /P $BIT 1024 /P $hBIT 512 /P $SZ 32 /P $hSZ 16 uintx.c.h ..\uint1024.c
C:\Factory\SubTools\rept.exe /P $BIT 1024 /P $hBIT 512 /P $SZ 32 /P $hSZ 16 uintx.h.h ..\uint1024.h

C:\Factory\SubTools\rept.exe /P $BIT 2048 /P $hBIT 1024 /P $SZ 64 /P $hSZ 32 uintx.c.h ..\uint2048.c
C:\Factory\SubTools\rept.exe /P $BIT 2048 /P $hBIT 1024 /P $SZ 64 /P $hSZ 32 uintx.h.h ..\uint2048.h

C:\Factory\SubTools\rept.exe /P $BIT 4096 /P $hBIT 2048 /P $SZ 128 /P $hSZ 64 uintx.c.h ..\uint4096.c
C:\Factory\SubTools\rept.exe /P $BIT 4096 /P $hBIT 2048 /P $SZ 128 /P $hSZ 64 uintx.h.h ..\uint4096.h

C:\Factory\SubTools\rept.exe /P $BIT 8192 /P $hBIT 4096 /P $SZ 256 /P $hSZ 128 uintx.c.h ..\uint8192.c
C:\Factory\SubTools\rept.exe /P $BIT 8192 /P $hBIT 4096 /P $SZ 256 /P $hSZ 128 uintx.h.h ..\uint8192.h
