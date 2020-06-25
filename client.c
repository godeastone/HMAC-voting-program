#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include<time.h>

#define BUF_SIZE 100
#define NORMAL_SIZE 20

void *sending_thread(void* arg);
void *receiving_thread(void* arg);

char name[NORMAL_SIZE]="[DEFALT]";     // name
char msg_form[NORMAL_SIZE];            // msg form
char serv_time[NORMAL_SIZE];        // server time
char msg[BUF_SIZE];                    // msg
char serv_port[NORMAL_SIZE];        // server port number
char clnt_ip[NORMAL_SIZE];            // client ip address

int main(int argc, char *argv[])
{
  int sock;
  struct sockaddr_in server_addr;
  pthread_t send_thread, receive_thread;
  void* thread_return;

  //Print USAGE If arguments are wrong
  if (argc!=4) {
      printf(" USAGE : %s <ip> <port> <name>\n", argv[0]);
      return EXIT_FAILURE;
  }

  //save ip & port number
  int PORT = atoi(argv[2]);
  char IP[30] = {0,};
  strcpy(IP, argv[1]);

  //create socket
  sock = socket(PF_INET, SOCK_STREAM, 0);

  //Set the options for socket
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(IP);
  server_addr.sin_port = htons(PORT);

  //ask connection to server
  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr))==-1) {
    perror("connection error!\n");
    return EXIT_FAILURE;
  }

  //create sending data thread
  //and create receving data thread
  pthread_create(&send_thread, NULL, sending_thread, (void*)&sock);
  pthread_create(&receive_thread, NULL, receiving_thread, (void*)&sock);
  pthread_join(send_thread, &thread_return);
  pthread_join(receive_thread, &thread_return);
  close(sock);
  return 0;
}

void *sending_thread(void* socket)
{
  int sock = *((int *)socket);
  char name_msg[NORMAL_SIZE+BUF_SIZE];
  char myInfo[BUF_SIZE];
  char* who = NULL;
  char temp[BUF_SIZE];

  sprintf(myInfo, "%s's join. IP_%s\n",name , clnt_ip);
  write(sock, myInfo, strlen(myInfo));

  while(1) {

      fgets(msg, BUF_SIZE, stdin);
      // menu_mode command -> !menu

      if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
      {
          close(sock);
          exit(0);
      }

      // send message
      sprintf(name_msg, "%s %s", name,msg);
      write(sock, name_msg, strlen(name_msg));
  }
  return NULL;
}


void *receiving_thread(void* socket)
{
  int sock = *((int*)socket);
  char name_msg[NORMAL_SIZE+BUF_SIZE];
  int str_len;

  while(1) {

    if(str_len = read(sock, name_msg, NORMAL_SIZE+BUF_SIZE-1) == -1) {
      perror("reading error!\n");
      return (void *)EXIT_FAILURE;
    }

    name_msg[str_len]=0;
    fputs(name_msg, stdout);
  }

  return NULL;
}

