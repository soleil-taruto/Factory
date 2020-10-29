rem ★Windows認証ver_レコード無しver
rem _makeDb_w2.bat SERVER DB
IF "%2" == "" GOTO END

> tbl.rec.tmp sqlcmd -S %1 -k2 -Q "select schema_id, name from [%2].[sys].[tables]"
> sch.rec.tmp sqlcmd -S %1 -k2 -Q "select schema_id, name from [%2].[sys].[schemas]"

C:\Factory\SQLSvr200x\export\MakeTableList.exe tbl.rec.tmp sch.rec.tmp %2 tbl.tmp

C:\Factory\Tools\RDMD.exe /RM out

FOR /F %%T IN (tbl.tmp) DO (

	> hdr.out.tmp sqlcmd -S %1 -k2 -Q "select * from %%T where 1 = 2"
	C:\Factory\SQLSvr200x\RecReader.exe hdr.out.tmp hdr.out.tmp

	rem bcp %%T out tbl.out.tmp -S %1 -T -c -t 0x01 -r 0x02
	rem C:\Factory\SQLSvr200x\BcpReader.exe 01 02 tbl.out.tmp tbl.out.tmp

	COPY /B hdr.out.tmp out\%%T.csv
	rem COPY /B hdr.out.tmp + tbl.out.tmp out\%%T.csv
)

:END
