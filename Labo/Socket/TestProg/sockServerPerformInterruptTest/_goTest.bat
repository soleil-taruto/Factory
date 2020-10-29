START Server.exe

FOR %%C IN (10, 15, 20, 25, 30) DO (
	START Client.exe %%C000000
)
