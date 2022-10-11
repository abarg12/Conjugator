#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFSIZE 450

struct __attribute__((__packed__)) header {
    short type;
    char source[20];
    char destination[20];
    int length;
    int message_id;
};

char *clid;
int sockfd;

void error(char *msg) {
    perror(msg);
    exit(1);
}

void program_loop();
void write_to_server(char *instr);
void send_header(short type, char *dest, int length, int msg_id);
void send_data(char *file_name);
void print_header(struct header *new_h);
void read_from_server(struct header *h);

int main(int argc, char** argv) {
    int portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];


    /* check command line arguments */
    if (argc != 4) {
       fprintf(stderr,"usage: %s <hostname> <port> <client id>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);
    clid = argv[3];

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

    printf("\n\n---------------------------------------------\n");
    printf("Welcome to the Ch477y C47hy messaging client!\nSeg faults included.\n");
    printf("---------------------------------------------\n\n");
    

    printf("Enter a message (<type> <dest> <length> <message id> <filename>): \n");
    program_loop();
    if(close(sockfd) < 0) {
        error("error closing socket");
    }
}


void program_loop() {
    char rbuff[450];
    int n;
    fd_set active_fd_set, read_fd_set;    
    FD_ZERO(&active_fd_set);
    FD_SET(0, &active_fd_set);
    FD_SET(sockfd, &active_fd_set);

    while (1) {
        read_fd_set = active_fd_set;

        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            error("Select() error");
        }
    
        int i;
        for (i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET (i, &read_fd_set)) {
                printf("socket: %d\n", i);
                if (i == 0) {
                    // stdin received
                    char *instr = malloc(100);
                    fgets(instr, 100, stdin);
                    
                    write_to_server(instr);

                }
                else {
                    n = read(i, rbuff, 50);
                    printf("you've read: %d\n", n);
                    char *b = malloc(50); 
                    memcpy(b, rbuff, 50);
                    struct header *h = (struct header *) b; 
                    print_header(h);
                    read_from_server(h);
                    free(h);
                }
            }
        }
        
        printf("Enter a message (<type> <dest> <length> <message id> <filename>): \n");
    }
}

void read_from_server(struct header *h) {
    h->type = ntohs(h->type);
    h->length = ntohl(h->length);
    h->message_id = ntohl(h->message_id); 

    printf("reading type: %d\n", h->type);
    if (h->type == 5) {
        
        char buff[h->length];
        int n = 0;
        while (n < h->length) {
            n = n + read(sockfd, buff, h->length);
        }        

        int i;
        for (i = 0; i < h->length; i++) {
            printf("%c", buff[i]);
        }
        printf("\n");
        printf("------End of message------\n");
    }
}

void write_to_server(char *instr) {
    char **chunks = malloc(sizeof(char*)*5);
    int i;
    int n = 0;
    
    char *token = strtok(instr, " ");
    chunks[0] = token; 
    for (i = 1; i < 5; i++) {
        if (token == NULL) {
            error("ill-formatted instructions provided");
        }
        chunks[i] = strtok(NULL, " "); 
    }
    /*for (i = 0; i < 5; i++) {
        printf(chunks[i]);
    }*/

    char *dest = malloc(20);
    char *file_name = malloc(20);
    bzero(dest, 20);
    bzero(file_name, 20);

    short type = htons(atoi(chunks[0]));
    memcpy(dest, chunks[1], strlen(chunks[1]));
    int length = htonl(atoi(chunks[2]));
    int msg_id = htonl(atoi(chunks[3])); 
    memcpy(file_name, chunks[4], strlen(chunks[4]));
    char *curr = file_name;
    while (*curr != '\0') {
        if (*curr == '\n' || *curr == '\n') {
            *curr = '\0'; 
        }
        curr++;
    }

    send_header(type, dest, length, msg_id);

    if (type == htons(1)) {
        char *mybuff = malloc(50);
        n = read(sockfd, mybuff, 50); 
        struct header *h = (struct header*) mybuff; 
        print_header(h);
        n = read(sockfd, mybuff, 50);
        h = (struct header*) mybuff;
        print_header(h);
        n = read(sockfd, mybuff, ntohl(h->length));
        for (i = 0; i < n; i++) {
            if (mybuff[i] == '\0') {
                printf(" | ");
            }
            printf("%c", mybuff[i]);
        }
        printf("\n");
    }
    
    if (type == htons(5)) {
        printf("trying to send data\n");
        send_data(file_name);
    }

    free(dest);
    free(file_name);
    free(chunks);
}




void send_header(short type, char *dest, int length, int msg_id) {
    struct header h;
    h.type = type;
    memcpy(h.source, clid, 20);
    memcpy(h.destination, dest, 20);
    h.length = length;
    h.message_id = msg_id;

    char *msg;
    msg = (char *) &h;

    write(sockfd, msg, 50);
}


void send_data(char *file_name) {
    printf(file_name);
    char file_buff[400];
    FILE *f = fopen(file_name, "rb");
    if (f == NULL) error("file read error\n");
    int num_read = fread(file_buff, 1, 400, f); 
    fclose(f);
    printf("%d\n",num_read); 
    write(sockfd, file_buff, num_read);
}


void print_header(struct header *new_h) {
    printf("------------Header------------\n");
    printf("type: %d\n", ntohs(new_h->type));
    printf("source: %s\n", new_h->source);
    printf("destination: %s\n", new_h->destination);
    printf("length: %d\n", ntohl(new_h->length));
    printf("message id: %d\n", ntohl(new_h->message_id));
    printf("------------------------------\n");
}
