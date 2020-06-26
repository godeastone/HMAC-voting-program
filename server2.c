#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#define BUF_SIZE 100
#define MAX_CLIENT 100
#define MAX_IP 30
#define CANDID_MAX 30

void *reading_function(void *sock);
void *writing_function(void *sock);
void survey_setting();
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

int flag = 1;
char candid_num[BUF_SIZE];
char main_subject[BUF_SIZE];
int client_num = 0;
int client_sockets[MAX_CLIENT];
int candid_num_int, duration_int;
int server_sock, client_sock;
char candid_name[BUF_SIZE], duration[BUF_SIZE];

//HMAC variables
unsigned char md[EVP_MAX_MD_SIZE];
int mdLen;
char *digest_name = "sha256";
char *key = "123412341234";
char message_hmac[BUF_SIZE];

struct candidates candid_list[CANDID_MAX];
struct sockaddr_in server_adr, client_adr;

pthread_mutex_t mutex;



int main(int argc, char *argv[])
{
  int address_size, mdLen;
  pthread_t thread_id, thread_id2;
  struct sigaction sig;

  //print USAGE when arguments are wrong
  if (argc != 2) {
    printf(" USAGE : %s <PORT>\n", argv[0]);
    return EXIT_FAILURE;
  }

  //assign argv[1] in variable PORT
  int PORT = atoi(argv[1]);

  //set signal handler
  sig.sa_handler = signal_handler;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
  sigaction(SIGALRM, &sig, NULL);

  //setting the survey
  survey_setting(PORT);

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
    address_size = sizeof(client_adr);
    //waiting for clients
    client_sock = accept(server_sock, (struct sockaddr*)&client_adr, &address_size);


    //lock
    pthread_mutex_lock(&mutex);
      //save client's socket in array 'client_socks'
    client_sockets[client_num++] = client_sock;
    //unlock
    pthread_mutex_unlock(&mutex);

    //create thread
    pthread_create(&thread_id, NULL, reading_function, (void *)&client_sock);
    pthread_detach(thread_id);
    pthread_create(&thread_id2, NULL, writing_function, (void *)&client_sock);
    pthread_detach(thread_id2);

    if(flag == 1)
      fprintf(stderr, "An ANONYMOUS(?) voter enters the survey\n");

    //close connection when timeout
    if(flag == 0)
      break;
  }

  return 0;
}


void *reading_function(void *sock)
{
  int client_sock = *((int*)sock);
  int len = 0;
  int choice;
  char message[BUF_SIZE];

  //read data from client
  while((len = read(client_sock, message, sizeof(message))) != 0) {

    choice = atoi(message);
    if(choice != 0) {
      fprintf(stderr, "  One voter picks : %d\n", choice);
      candid_list[choice-1].votes++;
      break;
    }

    if(flag == 0)
      break;
  }

  //save some values for HMAC
  strcpy(message_hmac, message);

  //calculate HMAC
  if(!create_HMAC(digest_name, md, &mdLen,
        key, strlen(key), message_hmac, strlen(message_hmac))) {
    perror("hmac error!\n");
  }

  fprintf(stderr, "HMAC = ");
  printhex(md, mdLen);


  // remove this client from client_sockets array
  pthread_mutex_lock(&mutex);
  for (int i = 0; i < client_num; i++) {
    if (client_sock == client_sockets[i]) {

      while(i++ < client_num-1) {
        client_sockets[i] = client_sockets[i+1];
      }
      break;
    }
  }

  client_num--;
  pthread_mutex_unlock(&mutex);

  fprintf(stderr, "An ANONYMOUS(?) voter finishes the survey\n");

  //close the client socket
  //thread terminate
  close(client_sock);

  return NULL;
}


void *writing_function(void *sock)
{
  int client_sock = *((int *)sock);
  char temp[BUF_SIZE];


  //show clients survey information
  //memset(temp, 0, BUF_SIZE);
  strcpy(temp, "******** SURVEY INFO ********\n");
  write(client_sock, temp, strlen(temp)+1);
  //memset(temp, 0, BUF_SIZE);
  strcpy(temp, "[Survey Topic] \n");
  write(client_sock, temp, strlen(temp)+1);
  write(client_sock, main_subject, strlen(main_subject)+1);
  //memset(temp, 0, BUF_SIZE);
  strcpy(temp, "\n***** SURVEY CANDIDATES *****\n\n");
  write(client_sock, temp, strlen(temp)+1);

  for(int i = 0; i < candid_num_int; i++) {

    //write(client_sock, "[\n", 1);
    //memset(temp, '\0', BUF_SIZE);
    //sprintf(temp, "%d", i);
    ///write(client_sock, temp, sizeof(temp)+1);
    //write(client_sock, "] \n", 1);
    //memset(temp, 0, BUF_SIZE);
    strcpy(temp, candid_list[i].name);
    write(client_sock, temp, sizeof(temp)+1);
    write(client_sock, "\n", 1);
  }

  //memset(temp, '\0', BUF_SIZE);
  strcpy(temp, "\n****************************\n\n");
  write(client_sock, temp, sizeof(temp));
  //memset(temp, '\0', BUF_SIZE);

  return NULL;
}


void survey_setting()
{
  char yn;

again:

  fprintf(stderr, "***********************\n");
  fprintf(stderr, "*   SURVEY SETTINGS   *\n");
  fprintf(stderr, "***********************\n");

  fprintf(stderr, "\n");
  fprintf(stderr, "* Please enter main subject of survey *\n");
  fprintf(stderr, "-> ");

  //Server user enters the main subject of the survey
  fgets(main_subject, BUF_SIZE, stdin);

  fprintf(stderr, "\n");
  fprintf(stderr, "* Please enter the number of candidates *\n");
  fprintf(stderr, "-> ");

  //Server user enters the number of candidates
  fgets(candid_num, BUF_SIZE, stdin);
  candid_num_int = atoi(candid_num);

  //error handling
  if(candid_num_int == 0) {
    fprintf(stderr, "\n");
    fprintf(stderr, "## wrong number!\n ##try again!\n");
    goto again;
  }

  //set candidates' information in candid_list
  for(int i = 0; i < candid_num_int; i++) {

    fprintf(stderr, "\n");
    fprintf(stderr, "   * Enter %d candidate's name *\n", i+1);
    fprintf(stderr, "   > ");

    fgets(candid_name, BUF_SIZE, stdin);

    strcpy(candid_list[i].name, candid_name);
    candid_list[i].votes = 0;
  }

  //show candidates
  system("clear");
  fprintf(stderr, "\n\n");
  fprintf(stderr, "******** SURVEY INFO ********\n\n");
  fprintf(stderr, "[Survey Topic] %s\n", main_subject);
  fprintf(stderr, "***** SURVEY CANDIDATES *****\n\n");
  for(int i = 0; i < candid_num_int; i++) {

    fprintf(stderr, "[%d] %s", i+1, candid_list[i].name);
  }
  fprintf(stderr, "\n");
  fprintf(stderr, "*****************************\n\n");

  //server user enters the duration minutes
  fprintf(stderr, "* Please enter duration of the survey (minutes) *\n");
  fprintf(stderr, "-> ");

  fgets(duration, BUF_SIZE, stdin);
  duration_int = atoi(duration);

  //set the alarm
  alarm(duration_int * 60);

  fprintf(stderr, "********************************\n");
  fprintf(stderr, "*         SURVEY START!        *\n");
  fprintf(stderr, "* SURVEY ends after %d minutes  *\n", duration_int);
  fprintf(stderr, "********************************\n");

  return;
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
  fprintf(stderr, "\n\n### Survey Time OUT ###\n");
  fprintf(stderr, "************ RESULT ***********\n\n");

  for(int i = 0; i < candid_num_int; i++) {
    fprintf(stderr, "*** [%d] %s   -> %d votes\n\n",
        i+1, candid_list[i].name, candid_list[i].votes);
  }
  fprintf(stderr, "*******************************\n");

  flag = 0;

  //end connection
  close(server_sock);
}
