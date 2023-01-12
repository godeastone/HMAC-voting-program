# VottingProgram_with_C

* If you compile this codes (server.c, client.c), two program will created; **SERVER** and **CLIENT**

* You can easily compile codes by just enter `make` on terminal, because of the Makefile.

* First, enter the PORT number after the SERVER execution file. `ex) ./SERVER 5656`. Sever can create the survey. Enter the main topic of the survey, number of the entry, name of the entry, and duration time as the program guide. Then, ready for survey is complete. after the duration time, the survey will closed and print the result on server's screen.

* Next, enter the IP address of server and PORT number after the CLIENT execution file, `ex) ./CLIENT 123.456.789.112 5656` client can see the information about survey, and choose the one entry. Then, the client program will generate HMAC by the value that you chose. then, client send the your choice and HMAC to server.

* Finally, Server compare gernerate HMAC and received HMAC. if they are same, the Mssage Authentication was complete.

* If you choose wrong number or HMAC was not same, your choice is not reflected in result.

* Use `apt-get install libssl-dev` when you cannot compile
