start tunnel 80 localhost 81
start boomClient 81 localhost 60000 /ss 120 /rs 120
start tunnel 60000 localhost 60001
start boomServer 60001 localhost 8081 /ss 120 /rs 120
start tunnel 8081 localhost 8080
start ..\fileStore /p 8080
