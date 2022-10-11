/* 
 *  * tcpclient.c - A simple TCP client
 *   * usage: tcpclient <host> <port>
 *    */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFSIZE 1024

struct __attribute__((__packed__)) header {
    short type;
    char source[20];
    char destination[20];
    int length;
    int message_id;
};


/* 
 *  * error - wrapper for perror
 *   */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

    /* check command line arguments */
    if (argc != 4) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);
    char *clid = argv[3];

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
      (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
      error("ERROR connecting");


    /* get message line from the user */
    // printf("Please enter msg: ");

    // fgets(buf, BUFSIZE, stdin);
    struct header *h = malloc(sizeof(*h));
    h->type = htons(1);
    strcpy(h->source, clid);
    strcpy(h->destination, "Server");
    h->length = htonl(0);
    h->message_id = htonl(1);
    char *msg;
    msg = (char *) h;

    /* send the message line to the server */
    n = write(sockfd, msg, 50);
    if (n < 0) 
      error("ERROR writing to socket");

    /* print the server's reply */ 
    bzero(buf, BUFSIZE);
    n = read(sockfd, buf, 50);
    if (n < 0) 
      error("ERROR reading from socket");

    h = (struct header *) buf;
     

    printf("Echo from server: %d, %s, %s, %d, %d\n\n", ntohs(h->type), h->source, h->destination, ntohl(h->length), ntohl(h->message_id));

    if (ntohs(h->type) == 7) {
        printf("epic fail\n");
        exit(1);
    }

    n = read(sockfd, buf, 50);
    h = (struct header *) buf;
    printf("Echo from server: %d, %s, %s, %d, %d\n\n", ntohs(h->type), h->source, h->destination, ntohl(h->length), ntohl(h->message_id));

    sleep(1);

    n = read(sockfd, buf, BUFSIZE);

    int j;
    for (j = 0; j < n; j++) {
        if (buf[j] == '\0') {
            printf("~");
        }
        else {
            printf("%c", buf[j]); 
        }
    }
    printf("\n");


    sleep(500);

// TODO: change the below message into list request
/*
    struct header *h = malloc(sizeof(*h));
    h->type = htons(1);
    strcpy(h->source, "Client1");
    strcpy(h->destination, "Server");
    h->length = htonl(0);
    h->message_id = htonl(1);
    char *msg;
    msg = (char *) h;

    n = write(sockfd, msg, 50);
    if (n < 0) 
      error("ERROR writing to socket");

    bzero(buf, BUFSIZE);
    n = read(sockfd, buf, BUFSIZE);
    if (n < 0) 
      error("ERROR reading from socket");

    h = (struct header *) buf;
 */    

    //printf("Echo from server: %d, %s, %s, %d, %d\n\n", ntohs(h->type), h->source, h->destination, ntohl(h->length), ntohl(h->message_id));


    //close(sockfd);
    //return 0;

}
