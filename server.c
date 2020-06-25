#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#define PORT 5678
#define MAX_CLIENTS 10
#define BUF_SIZE 1025

int main(int argc, char *argv[])
{
  int opt = 1;
  int master_socket, addrlen, new_socket, client_socket[MAX_CLIENTS];
  int activity, valread, sd, max_sd;
  struct sockaddr_in address;

  char buffer[BUF_SIZE];

  //set the socket
  fd_set readfds;

  char *message = "I'm server and obey to me\n";

  //initialize all client socket
  for(int i = 0; i < MAX_CLIENTS; i++) {
    client_socket[i] = 0;
  }

  //create master socket
  if((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket error!\n");
    exit(EXIT_FAILURE);
  }

  //set master socket to allow mutiple clients
  if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
    perror("setsockopt error!\n");
    exit(EXIT_FAILURE);
  }

  //set options of socket
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  //bind the sockt
  if(bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind error!\n");
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "connecting...\n");

  if(listen(master_socket, 3) < 0) {
    perror("listen error!\n");
    exit(EXIT_FAILURE);
  }

  //accept the conntection
  addrlen = sizeof(address);
  fprintf(stderr, "Waiting for connections...\n");

  while(1) {
    //clear socket set
    FD_ZERO(&readfds);

    //add master socket
    FD_SET(master_socket, &readfds);
    max_sd = master_socket;

    //add child sockets to set
    for(int i = 0; i < MAX_CLIENTS; i++) {
      //socket descriptor
      sd = client_socket[i];

      //if socket descriptor is valid add to read list
      if(sd > 0) {
        FD_SET(sd, &readfds);
      }

      //highest file descriptor number
      if(sd > max_sd) {
        max_sd = sd;
      }
    }

    //wait for activity of sockets
    activity = select(max_sd+1, &readfds, NULL, NULL, NULL);

    if((activity < 0) && (errno != EINTR)) {
      fprintf(stderr, "select error!\n");
    }

    //When clients active
    if(FD_ISSET(master_socket, &readfds)) {
      if((new_socket = accept(master_socket, (struct sockaddr *)&address,
              (socklen_t*)&addrlen)) < 0) {

        perror("accept error!\n");
        exit(EXIT_FAILURE);
      }

      fprintf(stderr, "New connection, socket fd : %d, ip : %s, port : %d\n",
          new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

      //send new connection greeting message
      if(send(new_socket, message, strlen(message), 0) != strlen(message)) {
        perror("send error!\n");
      }

      //add new socket to array of sockets
      for(int i = 0; i < MAX_CLIENTS; i++) {
        if(client_socket[i] == 0) {
          client_socket[i] = new_socket;
          fprintf(stderr, "Adding to list of sockets as %d\n", i);

          break;
        }
      }
    }

    //some IO operation on other socket
    for(int i = 0; i < MAX_CLIENTS; i++) {
      sd = client_socket[i];

      if(FD_ISSET(sd, &readfds)) {
        //check whether closed
        if((valread = read(sd, buffer, 1024)) == 0) {
          //show closed clients
          getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
          fprintf(stderr, "DISCONNECTED, ip : %s, port : %d\n",
              inet_ntoa(address.sin_addr), ntohs(address.sin_port));
          //close the socket
          close(sd);
          client_socket[i] = 0;
        } else {
          //echo back the message that came in
          buffer[valread] = '\0';
          send(sd, buffer, strlen(buffer), 0);
        }
      }
    }
  }

  return 0;
}
