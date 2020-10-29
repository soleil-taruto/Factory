start revserver 60000 a 1
start tunnel 60001 localhost 60000
start tunnel 60002 localhost 60000
start namedtrack 80 localhost 60001 cure:heart
start namedtrack 50000 localhost 60002 cure:diamond.R
start revClient localhost 50000 localhost 8080
start ..\fileStore /p 8080
