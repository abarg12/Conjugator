#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>

#include "client_list.h"


struct client_data {
    int valid;
    char *client_ID;
    int sock_fd;
    int port;
    int ip;
    
    enum client_state state;
    
    char *partial;
    int p_ind;
    char *msg_dest;
    int msg_size;
    int msg_id;
    time_t last_filled;

    struct client_data *next;
    struct client_data *prev;
};

struct client_list {
    client_data head; 
    client_data tail;
    int num_clients;
}; 



client_list make_client_list() {
    client_list cl = malloc(sizeof(*cl));
    cl->num_clients = 0;
    cl->head = NULL;
    cl->tail = NULL;

    return cl;
}

int num_clients(client_list cl) {
    return cl->num_clients;
}

void recursive_node_del(client_data curr) {
    if (curr->next != NULL) {
        recursive_node_del(curr->next);
    }
    free(curr->partial);
    if (curr->valid) {
        free(curr->client_ID); 
    }
    if (curr->msg_dest != NULL) {
        free(curr->msg_dest);
    }
    free(curr);
}

void delete_client_list(client_list cl) {
    recursive_node_del(cl->head);
    free(cl);
}


client_data get_by_fd(client_list cl, int fd) {
    client_data curr = cl->head;
    while (curr != NULL) {
        if (curr->sock_fd == fd) {
            return curr; 
        }
    }
    return NULL;
}

client_data get_by_addr(client_list cl, int ip, int port) {
    client_data curr = cl->head;
    while (curr != NULL) {
        if (curr->ip == ip) {
            if (curr->port == port) {
                return curr; 
            } 
        }
        curr = curr->next;
    }
    return NULL;
}


void create_new_client(client_list cl, struct sockaddr_in client_info, int sock_fd) {
    client_data new_client = malloc(sizeof(*new_client));
    new_client->valid = 0;
    new_client->client_ID = NULL;
    new_client->sock_fd = sock_fd;
    new_client->port = client_info.sin_port;
    new_client->ip = client_info.sin_addr.s_addr;

    new_client->state = READ_H;

    new_client->partial = malloc(900);
    bzero(new_client->partial, 900);
    new_client->p_ind = 0;
    new_client->msg_dest = NULL;
    new_client->msg_size = 0;
    new_client->msg_id = 0;
    new_client->last_filled = -1;

    if(cl->tail != NULL) {
        cl->tail->next = new_client; 
        new_client->prev = cl->tail;
    }
    else {
        cl->head = new_client; 
        new_client->prev = NULL;
    }
    cl->tail = new_client;
    new_client->next = NULL;
    cl->num_clients = cl->num_clients + 1;
}


void print_clients(client_list cl) {
    client_data curr = cl->head;
    while (curr != NULL) {
        printf("(valid?: %d, sock_fd: %d) ->", curr->valid, curr->sock_fd);
        curr = curr->next;
    }
    printf("\n");
}   

client_data fill_cl_partial(client_list cl, struct sockaddr_in cl_info, char *buffer, int bytes) {
    int i;

    client_data cd = get_by_addr(cl, cl_info.sin_addr.s_addr, cl_info.sin_port);
    if (cd == NULL) {
        perror("Client does not exist...");
        exit(1);  
    }

    char *cli_p = cd->partial;

    if (cd->last_filled > 0 && (time(NULL) - cd->last_filled) >= 60) {
        clear_partial(cd, cd->p_ind);
    }
    
    for (i = 0; i < bytes; i++) {
        cli_p[cd->p_ind] = buffer[i]; 
        cd->p_ind = cd->p_ind + 1;
    }

    cd->last_filled = time(NULL);
    return cd; 
}


int get_partial_ind(client_data cd) {
    if (cd->last_filled > 0 && (time(NULL) - cd->last_filled) >= 60) {
        clear_partial(cd, cd->p_ind);
    }
    return cd->p_ind;
}

char *get_partial_buff(client_data cd) {
    if (cd->last_filled > 0 && (time(NULL) - cd->last_filled) >= 60) {
        clear_partial(cd, cd->p_ind);
    }
    return cd->partial;
}

// n is the number of bytes at the front of buffer to clear
void clear_partial(client_data cd, int n) {
    char *lp = cd->partial;
    char *rp = cd->partial; 

    int i;
    for (i = n; i < 900; i++) {
        *lp = rp[i];
        lp++;
    }

    cd->p_ind = cd->p_ind - n; 
}


void delete_client(client_list cl, struct sockaddr_in cl_info) {
    client_data cd = get_by_addr(cl, cl_info.sin_addr.s_addr, cl_info.sin_port);
    if (cd == NULL) {
        perror("no such client to delete"); 
        exit(1);
    }
    free(cd->partial);
    if (cd->client_ID != NULL) free(cd->client_ID);
    
    if (cl->head == cd) {
        if (cd->next == NULL) {
            cl->head = NULL; 
        } else {
            cl->head == cd->next; 
        }
    }  
    if (cl->tail == cd) {
        if (cd->prev == NULL) {
            cl->tail = NULL; 
        } else {
            cl->tail = cd->prev;
        }
    }
    if (cd->prev != NULL) cd->prev->next = cd->next;
    if (cd->next != NULL) cd->next->prev = cd->prev;
    
    cl->num_clients = cl->num_clients - 1;
    free(cd);
}

// Set client as being validated and reorder to end of list to signify
// the order of clients arriving to the server
int validate_client(client_list cl, client_data cd, char *cli_id) {
    cd->valid = 1;
    cd->client_ID = cli_id;
    if (cd->prev == NULL) {
        if (cd->next != NULL) {
            cl->head = cd->next;
            cd->next->prev = NULL;
            cd->next = NULL;
            cd->prev = cl->tail;
            cl->tail->next = cd;
            cl->tail = cd;
        } else {
            return 0;
        }
    } else {
        if (cl->tail == cd) {
            return 0; 
        }
        cl->tail->next = cd;
        cd->prev = cl->tail;
        cl->tail = cd;
        cd->prev->next = cd->next;
        cd->next->prev = cd->prev;
        cd->next = NULL;
    }
}


client_data get_by_name(client_list cl, char *id) {
    client_data curr = cl->head;
    while (curr != NULL) {
        if (curr->valid == 1) {
            if (strcmp(curr->client_ID, id) == 0) {
                return curr;
            }
        }
        curr = curr->next;
    }
    return NULL;
}


int get_client_list(char *buffer, client_list cl) {
    bzero(buffer, 400);
    int b_ind = 0;
    
    client_data curr = cl->head;
    while (curr != NULL) {
       if (curr->valid == 1) {
            char *str_p = curr->client_ID;
            while (*str_p != '\0') {
                buffer[b_ind] = *str_p;
                b_ind++;
                str_p++;
            }
            buffer[b_ind] = '\0';
            b_ind++;
        } 
        curr = curr->next;
    }

    return b_ind;
}

enum client_state get_state(client_data cd) {
    return cd->state;
}

char *get_msg_dest(client_data cd) {
    return cd->msg_dest;
}

int get_sock_fd(client_data cd) {
    return cd->sock_fd;
}

void set_msg_data(client_data cd, int msg_id, int msg_size, char *msg_dest, enum client_state state) {
    cd->state = state;
    cd->msg_id = msg_id;
    cd->msg_size = msg_size;
    cd->msg_dest = msg_dest;  
}

int get_msg_size(client_data cd) {
    return cd->msg_size;
}

void set_state(client_data cd, enum client_state state) {
    cd->state = state;
}

char *get_client_id(client_data cd) {
    return cd->client_ID;
}

int get_msg_id(client_data cd) {
    return cd->msg_id;
}

