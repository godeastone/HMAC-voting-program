#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>


#define BUF_SIZE 100
#define NORMAL_SIZE 20

void *sending_thread(void* arg);
void *receiving_thread(void* arg);
void signal_handler(int sig_num);

char name[NORMAL_SIZE]="[DEFALT]";     // name
char msg_form[NORMAL_SIZE];            // msg form
char serv_time[NORMAL_SIZE];        // server time
char msg[BUF_SIZE];                    // msg
char serv_port[NORMAL_SIZE];        // server port number
char clnt_ip[NORMAL_SIZE];            // client ip address
int flag = 0;

int main(int argc, char *argv[])
{
  int sock;
  struct sockaddr_in server_addr;
  struct sigaction sig;
  pthread_t send_thread, receive_thread;
  void* thread_return;

  //Print USAGE If arguments are wrong
  if (argc != 3) {
      printf(" USAGE : %s <ip> <port>\n", argv[0]);
      return EXIT_FAILURE;
  }

  //save ip & port number
  int PORT = atoi(argv[2]);
  char IP[30] = {0,};
  strcpy(IP, argv[1]);

  //set signal handler
  sig.sa_handler = signal_handler;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
  sigaction(SIGALRM, &sig, NULL);

  //create socket
  sock = socket(PF_INET, SOCK_STREAM, 0);

  //Set the options for socket
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(IP);
  server_addr.sin_port = htons(PORT);

  //ask connection to server
  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    perror("connection error!\n");
    return EXIT_FAILURE;
  }

  char message[BUF_SIZE];
  int str_len;

  //create sending data thread
  //and create receving data thread
  pthread_create(&send_thread, NULL, sending_thread, (void*)&sock);
  //pthread_create(&receive_thread, NULL, receiving_thread, (void*)&sock);
  pthread_join(send_thread, &thread_return);
  //pthread_join(receive_thread, &thread_return);
  fprintf(stderr, "Complete survey!\n");

  //close the socket
  close(sock);
  return 0;
}

void *sending_thread(void* socket)
{
  sleep(1);
  int sock = *((int *)socket);
  char message[BUF_SIZE];
  char* who = NULL;
  char temp[BUF_SIZE];


  fgets(temp, BUF_SIZE, stdin);

  if (!strcmp(temp, "q\n") || !strcmp(temp, "Q\n")) {
    close(sock);
    exit(0);
  }

  // send message
  //memset(message, '\0', BUF_SIZE);
  sprintf(message, "%s", temp);
  write(sock, message, strlen(message)+1);

  return NULL;
}


void *receiving_thread(void* socket)
{
  /*
  int sock = *((int*)socket);
  char message[BUF_SIZE];
  int str_len;

  while(1) {

    //memset(message, '\0', BUF_SIZE);
    if(str_len = read(sock, message, BUF_SIZE) == -1) {
      perror("reading error!\n");
      return (void *)EXIT_FAILURE;
    }
    message[str_len+1] = '\0';
    fprintf(stderr, "%s", message);
  }
  return NULL;
  */
  int sock=*((int*)socket);
  char name_msg[NORMAL_SIZE+BUF_SIZE];
  int str_len;

  //set the alarm for terminate thread
  //alarm(3);

  while(1)
  {
    str_len=read(sock, name_msg, NORMAL_SIZE+BUF_SIZE-1);
    if (str_len==-1)
        return (void*)-1;
    name_msg[str_len]=0;
    fprintf(stderr, "%s", message);
  }

  return NULL;
}

void signal_handler(int sign_num)
{
  pthread_exit(NULL);
}
