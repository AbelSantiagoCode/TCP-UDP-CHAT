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
  
#define TRUE   1
#define FALSE  0
#define PORT 51717
#define FDLEN 10
 
static void error(const char *msg);
static void CreateTcpSock(int *sockfd, const char * ip, const char * portstr, struct sockaddr_in *serv_addr);
static void ConfigBindSock(int *sockfd,int numclient, struct sockaddr_in * serv_addr );
static void AcceptClient(int * sockfd, int * newsockfd,struct sockaddr_in * cli_addr);
static void UpdateSelect(fd_set * readfds,int * sockfd,int * fdlist);
static void CloseSock(int * sockfd, int * fdlist);
static void CheckExit(const char * buffer,int * sockfd, int * fdlist);
static void ClientToTerminal(int * sockfd,int * newsockfd,int indexsock, int * fdlist);
static void TerminalToClient(int * sockfd,int * fdlist);



static void error(const char *msg)
{
    perror(msg);
    exit(1);
}




static void CreateTcpSock(int *sockfd, const char * ip, const char * portstr, struct sockaddr_in *serv_addr){
    int portno;
    // CREATE SERVER SOCKET
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0) error("ERROR opening socket");
    // INITIALIZE SERVER SOCKET: PORT NUMBER, TYPE DOMAIN, PROTOCOL
    bzero((char *) serv_addr, sizeof(*serv_addr));
    portno = atoi(portstr);
    (*serv_addr).sin_family = AF_INET;
    inet_pton(AF_INET,ip, &((*serv_addr).sin_addr));
    (*serv_addr).sin_port = htons(portno);
    
}

static void ConfigBindSock(int *sockfd,int numclient, struct sockaddr_in * serv_addr ){
    // BIND BETWEEN SERVER-SOCKET AND PORT
    if (bind(*sockfd, (struct sockaddr *) serv_addr,sizeof(*serv_addr)) < 0) 
        error("ERROR on binding");
    // LISTING CLIENTS - CONFIGURE NUMBER OF CLIENTS 
    listen(*sockfd,numclient);
}

static void AcceptClient(int * sockfd, int * newsockfd,struct sockaddr_in * cli_addr){
    socklen_t clilen;
    // ACCEPT CLIENT IS COMING 
    clilen = sizeof(*cli_addr);
    *newsockfd = accept(*sockfd,(struct sockaddr *) cli_addr,&clilen);
    if (*newsockfd < 0) error("ERROR on accept");
}

static void UpdateSelect(fd_set * readfds,int * sockfd,int * fdlist){
    int max_sd,activity;
    // CLEAR THE SOCKET SET
    FD_ZERO(readfds);
    // ADD KEYBOARD TO FDLIST
    FD_SET(STDIN_FILENO,readfds);
    FD_SET(*sockfd,readfds);
    max_sd = *sockfd;

    for (uint8_t i = 0; i < FDLEN; i++){
        // ADD CLIENT SOCKET
        if (fdlist[i]!=0){
            FD_SET(fdlist[i],readfds);
            if (fdlist[i] > max_sd){
                max_sd = fdlist[i];
            }
        }
    }

    // Wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
    activity = select( max_sd + 1 , readfds , NULL , NULL , NULL);
    if ((activity < 0) && (errno!=EINTR)) {
        printf("select error");
    }
}


static void CloseSock(int * sockfd, int * fdlist){
    for (uint8_t i = 0; i<FDLEN; i++){
        if(fdlist[i]!=0) close(fdlist[i]);
    }
    close(*sockfd);
}

static void CheckExit(const char * buffer,int * sockfd, int * fdlist){
    char exit_l[15]="exit\n\0";
    if (strcmp(exit_l,buffer) == 0){
        CloseSock(sockfd,fdlist);
        exit(EXIT_SUCCESS);
    }
}


static void ClientToTerminal(int * sockfd,int * newsockfd,int indexsock, int * fdlist){
    char buffer[256];
    int n;
    //READ DATA SENT BY CLIENT
    bzero(buffer,256);
    n = read(*newsockfd,buffer,255);
    if (n < 0) {
        error("ERROR reading from socket");
        close(*newsockfd);
        close(*sockfd);
        exit(EXIT_FAILURE);
    }
    printf("\n%s\n",buffer);


    // TANQUEM EL SOCKET CLIENT SI DIU EXIT
    char exit_l[15]="exit\n\0";
    if (strcmp(exit_l,buffer) == 0){
        close(*newsockfd);
        fdlist[indexsock]=0;
    }

}

static void TerminalToClient(int * sockfd,int * fdlist){
    char buffer[256];
    int n;
    // Read KEYBOARD
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    // SEND DATA FROM TERMINAL TO CLIENT -BROADCAST

    for (uint8_t i = 0; i<FDLEN; i++){
        if (fdlist[i]!=0){
            n = write(fdlist[i],buffer,strlen(buffer));
            if (n < 0) error("ERROR writing to socket");
        }
    }
    CheckExit(buffer,sockfd,fdlist);
}





void main(int argc, char *argv[])
{
    int sockfd, newsockfd;
    int fdlist[10];
    struct sockaddr_in serv_addr, cli_addr;
    fd_set readfds;
    char ip[18]="127.0.0.1";


    // CHECK FOR PARAMETER INPUTS BY TERMINAL 
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    //initialise all client_socket[] to 0 so not checked
    for (uint8_t i = 0; i < FDLEN; i++){
        fdlist[i] = 0;
    }

    // INITIALIZE CONFIGURATION OF SOCKETS CLIENT-SERVER 
    CreateTcpSock(&sockfd,ip,argv[1],&serv_addr);
    ConfigBindSock(&sockfd,5,&serv_addr);



   // AcceptClient(&sockfd,&newsockfd,&cli_addr);
   
    // CHAT BETWEEN CLIENT-SERVER  
    while (TRUE){
        UpdateSelect(&readfds,&sockfd,fdlist);
        //FD_SET(sockfd,&readfds);

        //If something happened on the client-socket
        if (FD_ISSET(sockfd, &readfds)){

            AcceptClient(&sockfd,&newsockfd,&cli_addr);
            for (uint8_t i = 0; i < FDLEN; i++){
                if(fdlist[i] == 0){
                    fdlist[i]=newsockfd;
                    break;
                }
            }


        } else if (FD_ISSET(STDIN_FILENO, &readfds)){
            TerminalToClient(&sockfd,fdlist);
        } else {
            for (uint8_t i = 0; i < FDLEN; i++){
                if (FD_ISSET(fdlist[i],&readfds)){
                    ClientToTerminal(&sockfd,&(fdlist[i]),i,fdlist);
                }
            }
        }

    }


    // CLOSE SOCKETS OF CLIENT-SERVER
    CloseSock(&sockfd,&newsockfd);
    exit(EXIT_SUCCESS);
}

