#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>


#define BUF_SIZE 300
#define CANDID_MAX 30

void *sending_thread(void* arg);
void *receiving_thread(void* arg);
void signal_handler(int sig_num);
void printhex(unsigned char *b, int bLen);
int create_HMAC(const char *mdname, unsigned char *md, int *mdLen,
		const unsigned char *key, const int keyLen,
		const unsigned char *m, const int mLen);

struct candidates
{
  char name[BUF_SIZE];
  int votes;
};

int flag3 = 0;
pthread_mutex_t mutex;

//HMAC variables
unsigned char md[EVP_MAX_MD_SIZE];
int mdLen;
char *digest_name = "sha1";
char *key = "123412341234";
char message_hmac[BUF_SIZE];
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

  //Initialize pthread
  pthread_mutex_init(&mutex, NULL);

  //set signal handler
  sig.sa_handler = signal_handler;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
  sigaction(SIGALRM, &sig, NULL);

  system("clear");
  fprintf(stderr, "* Start survey! *\n");

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
  pthread_create(&receive_thread, NULL, receiving_thread, (void*)&sock);
  pthread_join(receive_thread, &thread_return);
  pthread_create(&send_thread, NULL, sending_thread, (void*)&sock);
  pthread_join(send_thread, &thread_return);
  fprintf(stderr, "Complete survey!\n");

  //close the socket
  close(sock);
  return 0;
}

void *sending_thread(void* socket)
{
  int sock = *((int *)socket);
  char message[BUF_SIZE];
  char temp[BUF_SIZE];

  while(1) {
    pthread_mutex_lock(&mutex);
    if(flag3 = 1) break;
    pthread_mutex_unlock(&mutex);
  }
  pthread_mutex_unlock(&mutex);

again:
  //fgets(temp, BUF_SIZE, stdin);
  fprintf(stderr, "Please Vote one of the candidates\n => ");
  scanf("%s", temp);

  if(atoi(temp) == 0 || atoi(temp) > CANDID_MAX) {
    fprintf(stderr, "## Please enter right number\n");
    goto again;
  }
  //save some values for HMAC
  strcpy(message_hmac, temp);

  //calculate HMAC
  if(!create_HMAC(digest_name, md, &mdLen,
        key, strlen(key), message_hmac, strlen(message_hmac))) {
    perror("hmac error!\n");
  }

  fprintf(stderr, "generated HMAC = ");
  printhex(md, mdLen);

  strcat(temp, " ");
  strcat(temp, md);

  //send message and HMAC
  write(sock, temp, strlen(temp)+1);

  return NULL;
}


void *receiving_thread(void* socket)
{
  struct candidates candid_list[CANDID_MAX];
  int sock=*((int*)socket);
  char message[BUF_SIZE];
  int str_len = 0;
  char main_subject[BUF_SIZE];
  int num = 0;
  int flag2 = 1;

  //set the alarm for terminate thread
  //alarm(3);

  //while(1) {

    //read survey information from server
    str_len = read(sock, message, BUF_SIZE);
    message[str_len] = '\0';

    if (str_len==-1) {

      perror("read error!\n");
      return (void *)EXIT_FAILURE;
    }

    //fprintf(stderr, "->%s\n", message);

    //split the data and save it
    char *ptr = strtok(message, "@");
    strcpy(main_subject, ptr);


    while(ptr != NULL) {

      if(flag2) {
        strcpy(main_subject, ptr);
        flag2 = 0;
      } else {
        strcpy(candid_list[num].name, ptr);
        num++;
      }
      ptr = strtok(NULL, "@");
    }

  pthread_mutex_lock(&mutex);

  //print client about survey info
  fprintf(stderr, "\n\n");
  fprintf(stderr, "******** SURVEY INFO ********\n\n");
  fprintf(stderr, "[Survey Topic] %s\n\n", main_subject);
  fprintf(stderr, "***** SURVEY CANDIDATES *****\n\n");

  for(int i = 0; i < num; i++) {

    fprintf(stderr, "[%d] %s\n", i+1, candid_list[i].name);
  }

  fprintf(stderr, "\n");
  fprintf(stderr, "*****************************\n\n");

  flag3 = 1;

  pthread_mutex_unlock(&mutex);

  //}
  return NULL;
}


void printhex(unsigned char *b, int bLen)
{
	fprintf(stderr, "%02X",b[0]);

	for(int i=1; i<bLen; i++) {
		fprintf(stderr, ":%02X", b[i]);
  }

	fprintf(stderr, "\n");
}


int create_HMAC(const char *mdname, unsigned char *md, int *mdLen,
    const unsigned char *key, const int keyLen,
    const unsigned char *m, const int mLen)
{
  HMAC_CTX *hctx;
  hctx = HMAC_CTX_new();

  //confirm HMAC_CTX is right
  if(hctx == NULL) {
    HMAC_CTX_free(hctx);
  }
  const EVP_MD *evpmd;

  OpenSSL_add_all_digests();

  if(!(evpmd = EVP_get_digestbyname(mdname))) {
    fprintf(stderr, "EVP_get_digestbyname error!\n");
    exit(0);
  }

  HMAC_Init_ex(hctx, key, keyLen, evpmd, NULL);
  HMAC_Update(hctx, m, mLen);
  HMAC_Final(hctx, md, mdLen);

  HMAC_CTX_free(hctx);

  return 1;
}

void signal_handler(int sign_num)
{
  pthread_exit(NULL);
}
