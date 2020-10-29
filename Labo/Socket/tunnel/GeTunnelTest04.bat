start tunnel 80 localhost 81
start boomClient 81 localhost 60000
start tunnel 60000 localhost 60001
start GeTunnel 60001 localhost 60002 /e 4
start tunnel 60002 localhost 60003
start GeTunnel 60003 localhost 60004 /r
start tunnel 60004 localhost 60005
start boomServer 60005 localhost 8081
start tunnel 8081 localhost 8080
start ..\fileStore /p 8080
