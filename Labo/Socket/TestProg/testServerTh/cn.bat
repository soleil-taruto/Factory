IF "%1" == "" GOTO END
FOR /L %%C IN (1, 1, %1) DO START Client.exe /R %%C
:END
