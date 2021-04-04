## VottingProgram_with_C

# ENGLISH


* If you compile this codes (server.c, client.c), two program will created **SERVER** and **CLIENT**

* You can easily compile codes by just enter `make` on terminal, because of the Makefile.

* First, enter the PORT number after the SERVER execution file. `ex) ./SERVER 5656`. Sever can create the survey. Enter the main topic of the survey, number of the entry, name of the entry, and duration time as the program guide. Then, ready for survey is complete. after the duration time, the survey will closed and print the result on server's screen.

* Next, enter the IP address of server and PORT number after the CLIENT execution file, `ex) ./CLIENT 123.456.789.112 5656` client can see the information about survey, and choose the one entry. Then, the client program will generate HMAC by the value that you chose. then, client send the your choice and HMAC to server.

* Finally, Server compare gernerate HMAC and received HMAC. if they are same, the Mssage Authentication was complete.

* If you choose wrong number or HMAC was not same, your choice is not reflected in result.

* Use `apt-get install libssl-dev` when you cannot compile


# KOREAN
* 이 코드를 컴파일 하면 두개의 프로그램, SERVER 와 CLIENT 가 생성됩니다.
코드를 컴파일 할때는 터미널에 `make`를 입력하면, 자동으로 컴파일되어 실행파일이 생성됩니다. (혹은 따로 server.c 와 client.c를 컴파일 해도 됩니다.)

* 먼저 SERVER 뒤에 포트 번호를 입력하여 실행시키면, `ex) ./SERVER 5656` 서버가 실행됩니다. 서버는 투표를 생성할 수 있습니다. 프로그램에서 안내하는데로 투표의 주제와, 후보 항목의 개수, 후보 항목을 입력하면 투표가 시작할 준비가 완료됩니다.

* 마지막으로, 투표 진행 시간을 입력하면 투표가 시작됩니다. 진행시간이 끝나면 자동으로 투표가 종료되고 결과를 출력합니다.


* 다음으로 CLIENT 를 서버의 ip주소와 포트번호와 함께 실행시키면, `ex) ./CLIENT 123.456.789.112 5656` 클라이언트가 실행됩니다. 클라이언트는 투표하려는 후보의 번호를 입력하면 자동으로 HMAC의 값이 출력되고, 종료됩니다.

* 만약 컴파일이 되지 않는다면 터미널에 `apt-get install libssl-dev`를 입력해 보십시오.
