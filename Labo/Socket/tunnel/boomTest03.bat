start boomClient 80 localhost 60000 /ss 1020 /rs 1020
start boomServer 60000 localhost 60001 /ss 1020 /rs 1020
start boomClient 60001 localhost 60002 /ss 21 /rs 21
start boomServer 60002 localhost 8080 /ss 21 /rs 21
start ..\fileStore /p 8080
