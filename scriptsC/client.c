#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 


#define TRUE   1
#define FALSE  0
#define PORT 51717


void error(const char *msg);
static void CheckExit(const char *buffer,int sockfd);
static void ReadServer(int sockfd);
static void ClientToServer(int sockfd);
static void UpdateSelect(fd_set * readfds, int sockfd);





void error(const char *msg)
{
    perror(msg);
    exit(0);
}

static void CheckExit(const char *buffer,int sockfd){    
    char str1[15]="exit\n\0";
    if (strcmp(str1,buffer) == 0){
        close(sockfd);
        exit(EXIT_FAILURE);

    }
}
static void ReadServer(int sockfd){
    char buffer[256];
    bzero(buffer,256);
    int n;
   
    n = read(sockfd,buffer,255);
    if (n < 0) {
        error("ERROR reading from socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    CheckExit(buffer,sockfd);
    printf("\n%s\n",buffer);
}

static void ClientToServer(int sockfd){
    // Read KEYBOARD-TERMINAL
    int n;
    char buffer[256];
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) error("ERROR writing to socket");
    CheckExit(buffer,sockfd);
}

static void UpdateSelect(fd_set * readfds, int sockfd){
    int max_sd,activity;

    // CLEAR THE SOCKET SET
    FD_ZERO(readfds);
    FD_SET(sockfd,readfds);
    FD_SET(STDIN_FILENO,readfds);
    max_sd = sockfd;
    // Wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
    activity = select( max_sd + 1 , readfds , NULL , NULL , NULL);
    if ((activity < 0) && (errno!=EINTR)) {
        printf("select error");
    }

}

void main(int argc, char *argv[])
{
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    //set of socket descriptors
    fd_set readfds;

    if (argc < 3) {
       fprintf(stderr,"usage %s ip port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)    error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr); //"192.168.0.163"
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");


    // CHAT CLIENT 
    while (TRUE){
        UpdateSelect(&readfds,sockfd);
        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(sockfd, &readfds)) {
            //READ SERVER
            ReadServer(sockfd);
        } else if (FD_ISSET(STDIN_FILENO, &readfds)){
            // Read KEYBOARD
            ClientToServer(sockfd);
        } else {
            printf("ERROR\n");
        }

    }
    // Close Programm
    close(sockfd);
    exit(EXIT_SUCCESS);
}