#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<time.h>

#define BUF_SIZE 100
#define MAX_CLIENT 100
#define MAX_IP 30

void *reading_function(void *sock);
void send_msg(char *msg, int len);
char *serverState(int count);
void menu(char port[]);


int client_num=0;
int client_socks[MAX_CLIENT];
pthread_mutex_t mutex;

int main(int argc, char *argv[])
{
    int server_sock, client_sock;
    struct sockaddr_in server_adr, client_adr;
    int client_adr_sz;
    pthread_t thread_id;

    //print USAGE when arguments are wrong
    if (argc != 2) {
      printf(" USAGE : %s <PORT>\n", argv[0]);
      return EXIT_FAILURE;
    }

    //assign argv[1] in variable PORT
    int PORT = atoi(argv[1]);

    //menu(argv[1]);
    //Initialize pthread
    pthread_mutex_init(&mutex, NULL);

    //create socket
    server_sock=socket(PF_INET, SOCK_STREAM, 0);

    //Set the options for socket
    memset(&server_adr, 0, sizeof(server_adr));
    server_adr.sin_family=AF_INET;
    server_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_adr.sin_port=htons(PORT);

    //binding the socket
    if (bind(server_sock, (struct sockaddr*)&server_adr, sizeof(server_adr)) == -1) {
      perror("bind error!\n");
      return EXIT_FAILURE;
    }

    //listening the connection
    if (listen(server_sock, 5) == -1) {
      perror("listen error!\n");
      return EXIT_FAILURE;
    }

    while(1) {
      client_adr_sz = sizeof(client_adr);
      client_sock = accept(server_sock, (struct sockaddr*)&client_adr, &client_adr_sz);

      pthread_mutex_lock(&mutex);
      //save client's socket in array 'client_socks'
      client_socks[client_num++] = client_sock;
      pthread_mutex_unlock(&mutex);

      pthread_create(&thread_id, NULL, reading_function, (void*)&client_sock);
      //pthread_detach(thread_id);
      fprintf(stderr, "Connceted client IP : %s \n", inet_ntoa(client_adr.sin_addr));
    }
    close(server_sock);
    return 0;
}

void *reading_function(void *sock)
{
  int client_sock = *((int*)sock);
  int len = 0;
  char message[BUF_SIZE];

  //read data from client
  while((len = read(client_sock, message, sizeof(message)))!=0) {
    fprintf(stderr, "From client : %s\n", message);
    memset(message, '\0', BUF_SIZE);
    //break;
  }

  // remove disconnected client
  pthread_mutex_lock(&mutex);
  for (int i = 0; i < client_num; i++)
  {
    if (client_sock==client_socks[i])
    {
        while(i++<client_num-1) {
          client_socks[i]=client_socks[i+1];
        }
        break;
      }
  }
  client_num--;
  pthread_mutex_unlock(&mutex);

  //close the client socket
  //thread terminate
  close(client_sock);

  return NULL;
}

void send_msg(char* msg, int len)
{
    int i;
    pthread_mutex_lock(&mutex);
    for (i=0; i<client_num; i++)
        write(client_socks[i], msg, len);
    pthread_mutex_unlock(&mutex);
}


char* serverState(int count)
{
    char* stateMsg = malloc(sizeof(char) * 20);
    strcpy(stateMsg ,"None");

    if (count < 5)
      strcpy(stateMsg, "Good");
    else
        strcpy(stateMsg, "Bad");

    return stateMsg;
}

void menu(char port[])
{
    system("clear");
    printf(" **** moon/sun chat server ****\n");
    printf(" server port    : %s\n", port);
    printf(" server state   : %s\n", serverState(client_num));
    printf(" max connection : %d\n", MAX_CLIENT);
    printf(" ****          Log         ****\n\n");
}
