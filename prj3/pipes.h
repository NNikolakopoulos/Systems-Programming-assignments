
struct sockaddr_in *create_sockaddr(char *hostname, int port);
void perror_exit(char *msg);
int create_client_socket(struct sockaddr_in *saddr);
int create_server_socket(struct sockaddr_in *saddr);
char *get_port_from_sock(int passive_sock_fd,struct sockaddr_in *passive_sa);
