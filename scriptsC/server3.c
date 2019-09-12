/**
    Handle multiple socket connections with select and fd_set on Linux
*/
  
#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <semaphore.h>

#define POSIX C SOURCE 200809L  
#define SHMOBJ_PATH  "/shm_AOS"
#define TRUE   1
#define FALSE  0
#define PORT 51717
#define FDLEN 10
#define NCLIENTS 10
#define ERR -1
#define OK 1
//gcc -std=gnu99 -o server3 server3.c -lrt -lpthread

static int create_shared_table(void);
static int bind_shared_table(void);
static void init_table(void);
static int remove_shared_table(void);
static void CloseSockPid(int sock);
static void kill_clients(int CloseServer);
static void error(const char *msg);
static void CreateServer(const char * ip, const char * portstr);
static void BindListenServer(void);
static void AcceptClient(int * newsockfd,struct sockaddr_in * cli_addr);
static void UpdateSelect(fd_set * readfds);
static void ServiceClient(int * newsockfd, struct sockaddr_in * CliAddr);
static void TerminalToClient(void);



/* CREATE AND CONFIGURE THE SHARED TABLE OBJECT */
/* ============================================ */

typedef struct{
  pid_t Pid;
  int   Sock;
  struct sockaddr_in Addr;
} client_t;

typedef struct{
  //sem_t sem_write;
  sem_t semph;//_read;
  int ServerSock;
  struct sockaddr_in serv_addr;
  client_t Client[NCLIENTS];

} table_t;



//SHARED VARIABLES
static int shmfd; //shared memory pointer to object shared.
static table_t *shmtable; //shared memory table.

static int create_shared_table(void){
  //CREATE A SHARED MEMORY OBJECT WITH FLAG O_CREAT
  shmfd = shm_open(SHMOBJ_PATH,O_CREAT|O_RDWR,S_IRWXU|S_IRWXG);//init
  if (shmfd == -1){
    return ERR;
  }

  //ALLOCATE SPACE TO SHMOBJECT, ONLY ONCE
  if (ftruncate(shmfd,sizeof(table_t)) == -1){
    return ERR;//    exit(EXIT_FAILURE);
  }
  if (close(shmfd) != 0) {
    return ERR;
  }
  return OK;
}


static int bind_shared_table(void){
  //OPEN SHMOBJECT WITH FLAG O_RDWR (READ-WRITE)
  shmfd = shm_open(SHMOBJ_PATH,O_RDWR,S_IRWXU|S_IRWXG);
  if (shmfd == -1)
    return ERR;
  //MAPPING THE SHMOBJECT TO MEMORY
  shmtable = (table_t *) mmap(NULL,sizeof(table_t),PROT_READ|PROT_WRITE,MAP_SHARED,shmfd,0);
  if ((shmtable == MAP_FAILED) || (close(shmfd) != 0)) {
    return ERR;
  }
  return OK;
}


static void init_table(void){
  //EMPTY THE SHARED TABLE
  shmtable->ServerSock = 0;  
  bzero((char *)&(shmtable->serv_addr), sizeof(shmtable->serv_addr));
  for (uint8_t i = 0; i < NCLIENTS; i++) {
    bzero((char *)&((shmtable->Client[i]).Addr), sizeof((shmtable->Client[i]).Addr));
    (shmtable->Client[i]).Pid = 0;
    (shmtable->Client[i]).Sock = 0;
  }

  //CREATE AND INITIALIZE SEMAPHORE WITH NAME
  //sem_init(&(shmtable->sem_write),1,1);
  sem_init(&(shmtable->semph),1,1);
}




/* FUNCTION SHARED TABLE OBJECT  */
/* ============================ */

static int remove_shared_table(void){
  //DESTROY SHMOBJ
  if (shm_unlink(SHMOBJ_PATH) == 0) {
    //ONLY IF A SEMAPHORE WAS CREATED
    if (sem_destroy(&(shmtable->semph)) == 0) {
      return OK;
    }
  }
  return ERR;
}

static void CloseSockPid(int sock){
  sem_wait(&(shmtable->semph));
  for (int i = 0; i < NCLIENTS; ++i){
    if ( (shmtable->Client[i]).Sock == sock ){
      bzero((char *)&((shmtable->Client[i]).Addr), sizeof((shmtable->Client[i]).Addr));
      (shmtable->Client[i]).Sock = 0;
      (shmtable->Client[i]).Pid = 0;
      close(sock);
      //kill((shmtable->Client[i]).Pid,SIGTERM);
      //return i;
    }
  }
  sem_post(&(shmtable->semph));
  exit(1);
 // return ERR;
}



static void kill_clients(int CloseServer){
  sem_wait(&(shmtable->semph));
    
  for (uint8_t i = 0; i < NCLIENTS ; i++) {
    if ((shmtable->Client[i]).Sock != 0){
      bzero((char *)&((shmtable->Client[i]).Addr), sizeof((shmtable->Client[i]).Addr));
      close((shmtable->Client[i]).Sock);
      kill((shmtable->Client[i]).Pid,SIGTERM);
      (shmtable->Client[i]).Pid = 0;
      (shmtable->Client[i]).Sock = 0;
    }
  }
  if (CloseServer){
    bzero((char *)&(shmtable->serv_addr), sizeof(shmtable->serv_addr));
    close(shmtable->ServerSock);
  }

  sem_post(&(shmtable->semph));

  if (CloseServer){
    remove_shared_table();
    exit(EXIT_SUCCESS);
  }
}



/* CREATE AND CONFIGURE SOCKET CLIENT-SERVER */
/* ========================================= */
static void error(const char *msg){
    perror(msg);
    exit(1);
}

static void CreateServer(const char * ip, const char * portstr){
  int portno;
  sem_wait(&(shmtable->semph));
  
  // CREATE SERVER SOCKET
  shmtable->ServerSock = socket(AF_INET, SOCK_STREAM, 0);
  if ((shmtable->ServerSock) < 0){
    error("ERROR opening socket");
  } 
  // INITIALIZE SERVER SOCKET: PORT NUMBER, TYPE DOMAIN, PROTOCOL
  bzero((char *)&(shmtable->serv_addr), sizeof(shmtable->serv_addr));
  portno = atoi(portstr);
  (shmtable->serv_addr).sin_family = AF_INET;
  inet_pton(AF_INET,ip, &((shmtable->serv_addr).sin_addr));
  (shmtable->serv_addr).sin_port = htons(portno);
  
  sem_post(&(shmtable->semph));    
}

static void BindListenServer(void){
  sem_wait(&(shmtable->semph));

  // BIND BETWEEN SERVER-SOCKET AND PORT
  if (bind(shmtable->ServerSock, (struct sockaddr *)&(shmtable->serv_addr), sizeof(shmtable->serv_addr)) < 0) 
    error("ERROR on binding");
  // LISTING CLIENTS - CONFIGURE NUMBER OF CLIENTS 
  listen(shmtable->ServerSock,NCLIENTS);

  sem_post(&(shmtable->semph));    
}


static void AcceptClient(int * newsockfd,struct sockaddr_in * cli_addr){
  sem_wait(&(shmtable->semph));

  socklen_t clilen;
  // ACCEPT CLIENT IS COMING 
  clilen = sizeof(*cli_addr);
  *newsockfd = accept((shmtable->ServerSock),(struct sockaddr *) cli_addr,&clilen);
  if (*newsockfd < 0) error("ERROR on accept");

  sem_post(&(shmtable->semph));    
}


/* FUNCTIONS IN REGARDING WITH SELECT */
/* ================================== */

static void UpdateSelect(fd_set * readfds){
  int sockfd;
  int max_sd,activity;

  sem_wait(&(shmtable->semph));
  sockfd = shmtable->ServerSock;
  sem_post(&(shmtable->semph)); 

  // CLEAR THE SOCKET SET
  FD_ZERO(readfds);
  // ADD KEYBOARD TO FDLIST
  FD_SET(STDIN_FILENO,readfds);
  FD_SET(sockfd,readfds);
  max_sd = sockfd;

  // Wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
  activity = select( max_sd + 1 , readfds , NULL , NULL , NULL);
  if ((activity < 0) && (errno!=EINTR)) {
    printf("select error");
  }
}


/* SERVICE TO CLIENT */
/* ================= */
static void ServiceClient(int * newsockfd, struct sockaddr_in * CliAddr){ 
  char exit_l[15]="exit\n\0";
  char buffer[256];
  int n;
  while (TRUE){
    //READ DATA SENT BY CLIENT
    bzero(buffer,256);
    n = read(*newsockfd,buffer,255);
    if (n < 0) {
      error("ERROR reading from socket");
      CloseSockPid(*newsockfd);
      exit(EXIT_FAILURE);
    }
    printf("\n%s\n",buffer);


    // TANQUEM EL SOCKET CLIENT SI DIU EXIT
    if (strcmp(exit_l,buffer) == 0){
      CloseSockPid(*newsockfd);
      //exit(EXIT_SUCCESS);
    }
  }
}


static void TerminalToClient(void){
  char exit_l[15]="exit\n\0";
  char buffer[256];
  int n;
  // Read KEYBOARD
  bzero(buffer,256);
  fgets(buffer,255,stdin);
  // SEND DATA FROM TERMINAL TO CLIENT -BROADCAST
  sem_wait(&(shmtable->semph));
  for (int i = 0; i < NCLIENTS; ++i){
    if ( (shmtable->Client[i]).Sock != 0 ){
      n = write((shmtable->Client[i]).Sock,buffer,strlen(buffer));
      if (n < 0) error("ERROR writing to socket");
    }
  }
  sem_post(&(shmtable->semph));

  if (strcmp(exit_l,buffer) == 0){
    kill_clients(1);
    exit(EXIT_SUCCESS);
  }
}


/* APPLICATION OF SERVE */
/* ==================== */
void main(int argc, char *argv[]){
  // VARIABLES
  int SockeServer, newsockfd;
  struct sockaddr_in CliAddr;
  fd_set readfds;
  pid_t Pid_Server = getpid();
  pid_t Pid_Client;
  char ip[18]="127.0.0.1";



  // CHECK FOR PARAMETER INPUTS BY TERMINAL 
  if (argc < 2) {
      fprintf(stderr,"ERROR, no port provided\n");
      exit(1);
  }


  // CREATE AND CONFIGURATION OF SHARED TABLE OBJECT
  create_shared_table();
  bind_shared_table();
  init_table();


  // CREATE AND CONFIGURATION OF SOCKETS CLIENT-SERVER 
  CreateServer(ip,argv[1]);
  BindListenServer();
  sem_wait(&(shmtable->semph));
  SockeServer = shmtable->ServerSock;
  sem_post(&(shmtable->semph));  


  // CHAT BETWEEN CLIENT-SERVER  
  while (TRUE){
    UpdateSelect(&readfds);
    //FD_SET(sockfd,&readfds);

    //If something happened on the client-socket
    if (FD_ISSET(SockeServer, &readfds)){
      newsockfd=0;
      bzero((char *)&(CliAddr), sizeof(CliAddr));

      AcceptClient(&newsockfd,&CliAddr);
      if ((Pid_Client = fork())==-1){
          printf("ERROR CREATING PROCESS\n");
      }

      if (Pid_Client == 0){
        // CHILD PROCESS
        ServiceClient(&newsockfd,&CliAddr);
        break;

      } else {
        // PARENT PROCESS
        sem_wait(&(shmtable->semph));
        for (int i = 0; i < NCLIENTS; ++i){
          if ((shmtable->Client[i]).Sock == 0){
            (shmtable->Client[i]).Sock = newsockfd;
            (shmtable->Client[i]).Pid  = Pid_Client;
            break;
          }
        }
        sem_post(&(shmtable->semph));  
      }

    } else if (FD_ISSET(STDIN_FILENO, &readfds)){
        TerminalToClient();
    } else {
      printf("ERROR DETECTED\n");
    }

  }


  // CLOSE SOCKETS OF CLIENT-SERVER
  if (getpid() == Pid_Server){
    kill_clients(1);
  }
  exit(EXIT_SUCCESS);
}

