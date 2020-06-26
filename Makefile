all : SERVER CLIENT

SERVER : SERVER.o
	gcc -o SERVER SERVER.o -pthread -lcrypto

CLIENT : CLIENT.o
	gcc -o CLIENT CLIENT.o -pthread -lcrypto

SERVER.o : server.c
	gcc -c -o SERVER.o server.c

CLIENT.o : client.c
	gcc -c -o CLIENT.o client.c

clean:
	rm *.o SERVER CLIENT
