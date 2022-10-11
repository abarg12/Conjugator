#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "client_list.h"

// Handle error messages
void error(char *msg) {
    perror(msg);
    exit(1);
}

int create_main_sock(int port);
int renew_fd_set(fd_set sock_set, int main_sock, client_list cl);
void select_loop(fd_set sock_set);
struct sockaddr_in connect_new_client();
struct header interpret_header(client_data cd);
void print_header(struct header *new_h);

void hello(struct header *new_h, client_data cd);
void hello_ack(struct header *new_h, char *cli_id);
void list_request(struct header *new_h, client_data cd);
void chat(struct header* h, client_data cd);
void error_present(struct header *new_h, client_data cd);
void client_exit(client_data cd); 
void error_deliver(client_data cd);

void check_ready_send(client_data cd);

int main_fd;
fd_set active_fd_set, read_fd_set;
client_list cl;
char *rbf = NULL;
int curr_client_fd;

struct __attribute__((__packed__)) header {
    short type;
    char source[20];
    char destination[20];
    int length;
    int message_id;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Please specify port number as only argument\n");
        exit(0);
    } 
   
    main_fd = create_main_sock(atoi(argv[1])); 
    cl = make_client_list();
    rbf = malloc(450);

    FD_ZERO(&active_fd_set);
    FD_SET(main_fd, &active_fd_set);
    
    int err_condition = 0;
    while (err_condition == 0) {
        read_fd_set = active_fd_set;
        select_loop(read_fd_set);
    }

    delete_client_list(cl);
    free(rbf);
}

int create_main_sock(int port) {
    // Create the server socket to receive requests 
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
        error("socket error");

    // Declare the client address, and initialize it blank
    struct sockaddr_in server_address;
    bzero((char *) &server_address, sizeof(server_address));
    
    // Initialize the server address struct
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the server socket to an address
    if (bind(server_sock, (struct sockaddr *) &server_address,
                                   sizeof(server_address)) < 0) {
        error("Error on binding");        
    }
    

    if (listen(server_sock, 5) < 0) {
        error("Listen() error");
    }

    return server_sock;
}

void select_loop(fd_set read_fd_set) {
    int bytes_read;
    int partial_bytes;
    struct sockaddr_in client_info;
    int client_name_size = sizeof(client_info);

    /*int t;
    for (t = 0; t < FD_SETSIZE; t++) {
        if (FD_ISSET(t, &read_fd_set)) {
            printf("%d\n", t);
        }
    }*/


    if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
        error("Select() error");
    }
    
    int i;
    for (i = 0; i < FD_SETSIZE; i++) {
        if (FD_ISSET (i, &read_fd_set)) {
             
            curr_client_fd = i;
            if (i == main_fd) {
                client_info = connect_new_client(); 
            }
            else {
                if (getpeername(i, (struct sockaddr*) &client_info, &client_name_size) < 0) {
                    error("getpeername() error");
                }
                
                // READ DATA FROM CLIENT ONLY HERE
                bytes_read = read(i, rbf, 450);

                if (bytes_read < 0) {
                    error("read() error");
                }
                if (bytes_read == 0) {
                    FD_CLR(i, &active_fd_set);
                    if (get_by_addr(cl, client_info.sin_addr.s_addr, client_info.sin_port) != NULL) {
                        delete_client(cl, client_info);
                    }
                    if (close(i) < 0) {
                        error("close() error");
                    }
                    continue;
                }
               
                
                if (get_by_addr(cl, client_info.sin_addr.s_addr, client_info.sin_port) == NULL) {
                    create_new_client(cl, client_info, i); 
                    client_data cd = fill_cl_partial(cl, client_info, rbf, bytes_read);
                    if (get_partial_ind(cd) >= 50) {
                        interpret_header(cd);
                    }
                } else {
                    client_data cd = fill_cl_partial(cl, client_info, rbf, bytes_read);
                    if (get_state(cd) == READ_H) {
                        if (get_partial_ind(cd) >= 50) {
                            interpret_header(cd);
                        }
                    } else if (get_state(cd) == READ_M) {
                        check_ready_send(cd);
                    }
                }
            }
        }
    }
    print_clients(cl);
}

struct sockaddr_in connect_new_client() {
    struct sockaddr_in client_name;
    int new_client_fd;
    int client_len = sizeof(client_name);
    new_client_fd = accept(main_fd, (struct sockaddr*) &client_name, 
                            &client_len);
    if (new_client_fd < 0) {
        error("Accept() error");
    }
    FD_SET(new_client_fd, &active_fd_set);
    return client_name;
}

struct header interpret_header(client_data cd) {
    char hbuf[50];
    char *p = get_partial_buff(cd);
    int i;
    for (i = 0; i < 50; i++) {
        hbuf[i] = p[i]; 
    }
    clear_partial(cd, 50);

    struct header *new_h;
    new_h = (struct header*) &hbuf;

    // convert everything back to host byte order
    new_h->type = ntohs(new_h->type);
    new_h->length = ntohl(new_h->length);
    new_h->message_id = ntohl(new_h->message_id);


    print_header(new_h);

    switch(new_h->type) {
        case 1:
            hello(new_h, cd); 
            break;
        case 3:
            list_request(new_h, cd); 
            break;
        case 5:
            chat(new_h, cd);
            break;
        case 6:
            client_exit(cd);
            break;
        default:
            error("case not handled... oops\n");
            
    } 

}

void hello(struct header *new_h, client_data cd) {
    char *dest = malloc(20);
    char *src = malloc(20); 
    memcpy(dest, new_h->destination, 20);
    memcpy(src, new_h->source, 20);
    
    client_data cli = get_by_name(cl, src);        
    if (cli == NULL) {
        validate_client(cl, cd, src); 
        free(dest); // don't free src since used in cd object
        hello_ack(new_h, src);
        list_request(new_h, cd);
    }
    else {
        free(dest);
        free(src);
        error_present(new_h, cd); 
    }

}

void hello_ack(struct header *new_h, char *cli_id) {
    struct header send_h;

    send_h.type = htons(2);
    send_h.length = htonl(0);
    send_h.message_id = htonl(0);
    bzero(send_h.source, 20);
    memcpy(send_h.source, "Server", 7);
    memcpy(send_h.destination, cli_id, 20);

    char* message;
    message = (char *) &send_h;
    write(curr_client_fd, message, 50);
}

void list_request(struct header *new_h, client_data cd) {
    char* buffer = malloc(400);
    int list_bytes = get_client_list(buffer, cl);    
    char *dest = malloc(20);
    memcpy(dest, new_h->source, 20);

    // write and send the header and then the data
    struct header send_h;
    send_h.type = htons(4);
    send_h.length = htonl(list_bytes);
    send_h.message_id = htonl(0);
    bzero(send_h.source, 20);
    memcpy(send_h.source, "Server", 7);
    memcpy(send_h.destination, dest, 20); 
    char* msg;
    msg = (char *) &send_h;
    write(curr_client_fd, msg, 50);

    write(curr_client_fd, buffer, list_bytes);

    free(buffer);
    free(dest);
}


void error_present(struct header *new_h, client_data cd) {
    struct header send_h;
    char *dest = malloc(20);
    memcpy(dest, new_h->source, 20);

    send_h.type = htons(7);
    send_h.length = htonl(0);
    send_h.message_id = htonl(0);
    bzero(send_h.source, 20);
    memcpy(send_h.source, "Server", 7);
    memcpy(send_h.destination, dest, 20);

    char* message;
    message = (char *) &send_h;
    write(curr_client_fd, message, 50);

    struct sockaddr_in client_info;
    int client_name_size = sizeof(client_info);

    if (getpeername(curr_client_fd, (struct sockaddr*) &client_info, &client_name_size) < 0) {
        error("getpeername() error");
    }

    FD_CLR(curr_client_fd, &active_fd_set);
    if (get_by_addr(cl, client_info.sin_addr.s_addr, client_info.sin_port) != NULL) {
        delete_client(cl, client_info);
    }
    if (close(curr_client_fd) < 0) {
        error("close() error");
    }

    free(dest);
}


void client_exit(client_data cd) {
    struct sockaddr_in client_info;
    int client_name_size = sizeof(client_info);

    if (getpeername(curr_client_fd, (struct sockaddr*) &client_info, &client_name_size) < 0) {
        error("getpeername() error");
    }

    FD_CLR(curr_client_fd, &active_fd_set);
    if (get_by_addr(cl, client_info.sin_addr.s_addr, client_info.sin_port) != NULL) {
        delete_client(cl, client_info);
    }
    if (close(curr_client_fd) < 0) {
        error("close() error");
    }
}


void chat(struct header* h, client_data cd) {
    // don't free msg_dest bc it's provided to client_data
    char *msg_dest = malloc(20);
    memcpy(msg_dest, h->destination, 20);

    set_msg_data(cd, h->message_id, h->length, msg_dest, READ_M);

    check_ready_send(cd);
}


void check_ready_send(client_data cd) {
    int msg_size = get_msg_size(cd);
    if (get_partial_ind(cd) >= msg_size) {
        char *msg = get_partial_buff(cd);
        client_data dest_client = get_by_name(cl, get_msg_dest(cd)); 
        if (dest_client != NULL) {

            struct header send_h;
            memcpy(send_h.source, get_client_id(cd), 20);
            memcpy(send_h.destination, get_client_id(dest_client), 20);

            send_h.type = htons(5);
            send_h.length = htonl(get_msg_size(cd));
            send_h.message_id = htonl(get_msg_id(cd));

            char* message;
            message = (char *) &send_h;
            write(get_sock_fd(dest_client), message, 50);

            write(get_sock_fd(dest_client), msg, msg_size);

        } else {
            error_deliver(cd);
        }
        
        clear_partial(cd, msg_size);
        set_state(cd, READ_H); 
    }
}

void error_deliver(client_data cd) {
    struct header send_h;
    char *dest = malloc(20);
    memcpy(dest, get_client_id(cd), 20);

    send_h.type = htons(8);
    send_h.length = htonl(0);
    send_h.message_id = htonl(get_msg_id(cd));
    bzero(send_h.source, 20);
    memcpy(send_h.source, "Server", 7);
    memcpy(send_h.destination, dest, 20);

    char* message;
    message = (char *) &send_h;
    write(curr_client_fd, message, 50);

    free(dest);
}

void print_header(struct header *new_h) {
    printf("------------Header------------\n");
    printf("type: %d\n", new_h->type);
    printf("source: %s\n", new_h->source);
    printf("destination: %s\n", new_h->destination);
    printf("length: %d\n", new_h->length);
    printf("message id: %d\n", new_h->message_id);
    printf("------------------------------\n");
}

