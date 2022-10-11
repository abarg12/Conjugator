#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#ifndef CLIENT_LIST_H
#define CLIENT_LIST_H

typedef struct client_list *client_list;
typedef struct client_data *client_data;

enum client_state {READ_H, INTP_H, READ_M, PROC_R};

client_list make_client_list();
int *get_client_fds(client_list cl);
int num_clients(client_list cl);
int get_client_list(char *buffer, client_list cl);

client_data get_by_fd(client_list cl, int fd);
client_data get_by_addr(client_list cl, int ip, int port);
client_data get_by_name(client_list cl, char *id);

void create_new_client(client_list cl, struct sockaddr_in client_info, int sock_fd); 
client_data fill_cl_partial(client_list cl, struct sockaddr_in cl_info, char *buffer, int bytes);
void delete_client_list(client_list cl);
void print_clients(client_list cl);
void delete_client(client_list cl, struct sockaddr_in cl_info);
int validate_client(client_list cl, client_data cd, char *cli_id);

int get_partial_ind(client_data cd);
char *get_partial_buff(client_data cd);
void clear_partial(client_data cd, int n); 
enum client_state get_state(client_data cd);
char *get_msg_dest(client_data cd);
int get_sock_fd(client_data cd);
int get_msg_size(client_data cd);
char *get_client_id(client_data cd);
int get_msg_id(client_data cd);

void set_msg_data(client_data cd, int msg_id, int msg_size, 
                 char *msg_dest, enum client_state state);
void set_state(client_data cd, enum client_state state);

#endif
