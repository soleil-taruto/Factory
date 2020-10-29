start tunnel 80 localhost 81

start /min crypTunnel 81 localhost 60000 *abc
start /min boomClient 60000 localhost 60001 /ss 2500
start /min GeTunnel 60001 localhost 60002 /e 2

start tunnel 60002 localhost 60003

start /min GeTunnel 60003 localhost 60004 /r
start /min boomServer 60004 localhost 60005
start /min crypTunnel 60005 localhost 8081 /r *abc

start tunnel 8081 localhost 8080

start /min ..\fileStore /p 8080
