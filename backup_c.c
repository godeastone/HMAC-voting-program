#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define BUF_SIZE 256

int main(int argv, char *argc[])
{
  int sd, len;
  char buf[BUF_SIZE];
  char message[1024] = {0x00, };
  struct sockaddr_in srv_addr;
  char *ip = "192.168.0.112";
  char *PORT = "5678";

  if((sd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket error\n");
    exit(1);
  }

  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_addr.s_addr = inet_addr(argc[1]);
  srv_addr.sin_port = htons(atoi(PORT));

  if(connect(sd, (struct sockaddr *)&srv_addr, sizeof(srv_addr) == -1)) {
    perror("bind error\n");
    exit(1);
  }

  strcpy(buf, "Unix Domain Socket Test Message\n");

  if(send(sd, buf, sizeof(buf), 0) == -1) {
    perror("sned error\n");
    exit(1);
  }

  if(read(sd, message, sizeof(message)-1) == -1) {
    perror("read error\n");
  }

  fprintf(stderr, "Message from server : %s\n", message);

  close(sd);

  return 0;

  close(sd);

  return 0;
}
