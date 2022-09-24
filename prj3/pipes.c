#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>

#include "types.h"

void perror_exit(char *msg) {
    perror(msg);
    exit(1);
}

struct sockaddr_in *create_sockaddr(char *hostname, int port) {
    struct sockaddr_in *saddr = calloc(1, sizeof(struct sockaddr_in));
    struct hostent *hent;
    hent = gethostbyname(hostname);
    if( hent == NULL) {
        fprintf(stderr,"gethostbyname error with h_errno=%d\n",h_errno);
        return NULL;
    }
    //inet_pton(AF_INET,ntohl(*(hent->h_addr_list[0])), &(saddr->sin_addr));
    struct in_addr **addr_list = (struct in_addr **) hent->h_addr_list;
    //saddr->sin_addr.s_addr = ntohl((*((struct in_addr*)hent->h_addr_list[0])).s_addr);
    int adr;
    memcpy(&adr, hent->h_addr_list[0], hent->h_length);
    saddr->sin_addr.s_addr = adr;
    //saddr->sin_addr.s_addr = ntohl((*(addr_list[0])).s_addr);
    saddr->sin_family = AF_INET;
    saddr->sin_port = htons(port);
    return saddr;
}


int create_client_socket(struct sockaddr_in *saddr) {
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror_exit("create_client_socket, error with socket");

    //struct sockaddr_in *saddr_ptr =(struct sockaddr *)&saddr ;
    do {}
    while(connect(sock,(struct sockaddr *)saddr, sizeof(*saddr)) < 0);

    return sock;
}



int create_server_socket(struct sockaddr_in *saddr) {
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror_exit("create_server_socket, error with socket");

    // we want to re-use sockets
    int reuse_addr = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) == -1)
        perror_exit("create_server_socket, error with setsockopt");
    //struct sockaddr_in *saddr_ptr =(struct sockaddr *)&saddr;
    if (bind(sock, (struct sockaddr *)saddr, sizeof(*saddr)) == -1)
        perror_exit("create_server_socket, error with bind");

    if (listen(sock, 100) == -1)
        perror_exit("create_server_socket, error with listen");
    return sock;
}

char *get_port_from_sock(int passive_sock_fd,struct sockaddr_in *passive_sa) {
    char *port_str = calloc(6,sizeof(char));
    unsigned int socklen = sizeof(*passive_sa);
    if( getsockname(passive_sock_fd,(struct sockaddr*)passive_sa,&socklen) < 0) {
        perror("getsockname error");
        return NULL;
    }
    snprintf(port_str,6,"%hu",ntohs(passive_sa->sin_port));
    return port_str;
}
