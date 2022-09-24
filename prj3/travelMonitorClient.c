#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include<sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <pthread.h>

#include "hash.h"
#include "bloom.h"
#include "list.h"
#include "pipes.h"
#include "msg.h"
#include "protocol.h"
#include "functions.h"
#include "logs.h"
#include "global.h"


int main(int argc, char **argv) {

    srand(time(NULL));

    long int bloomsize = 100000, socketBufferSize = 1024;
    unsigned int cyclicBufferSize = 2, numThreads = 3;
    int numMonitors = 2;
    char *input_dir=calloc(30,sizeof(char));
    //strcpy(input_dir,"./input_dir");



    for(int i=1; i<argc; i++)
    {
        if(!strcmp(argv[i],"-m"))     
        {
            numMonitors = atoi(argv[i+1]);
        }
        if(!strcmp(argv[i],"-s"))     
        {
            bloomsize = atol(argv[i+1]);
        }
        if(!strcmp(argv[i],"-b"))     
        {
            socketBufferSize = atol(argv[i+1]);
        }
        if(!strcmp(argv[i],"-i"))     
        {
            strcpy(input_dir,argv[i+1]);
        }
        if(!strcmp(argv[i],"-c"))     
        {
            cyclicBufferSize = atoi(argv[i+1]);
        }
        if(!strcmp(argv[i],"-t"))     
        {
            numThreads = atoi(argv[i+1]);
        }
    }

    int i;
    pid_t pid[numMonitors];
    char **args;
    char *socketbuf;
    bool updateDatabase = false, init_monitors = true, ready_for_bloomfilters = false;
    List *list, *clist;
    unsigned int *ports = calloc(numMonitors, sizeof(unsigned int)), port;

    add_global_monitor(0,0); // initialize global struct for parent, to save stats for later use
    delete_log_dir();
    for( i=0; i<numMonitors; i++) {

        pid[i] = fork();

        port = get_rand_port(ports,numMonitors);
        ports[i] = port; // save random generated port number
        if(pid[i] == 0) { //if its a child
            char *exec_args;
            exec_args = get_exec_args(exec_args, port, bloomsize, socketBufferSize, cyclicBufferSize, numThreads, input_dir, numMonitors, i);
            execlp("monitor","./monitor",exec_args,(char *) NULL);
        }

    }
    // get hostname
    char *hostname = calloc(512,sizeof(char));
    if (gethostname(hostname,512) < 0) {
        perror("gethostname :");
        exit(1);
    } 
    //saddrs refer to addresses of child servers, passive_sa refers to address of a passive socket 
    struct sockaddr_in **saddrs, *passive_sa;


    saddrs = calloc(numMonitors, sizeof(struct sockaddr_in*));

    int sock_fds[numMonitors];

    for(i=0; i<numMonitors; i++) {
        saddrs[i] = create_sockaddr(hostname,htons(ports[i]));
        sock_fds[i] = create_client_socket(saddrs[i]);

    }
    // create a list for storing the requests
    clist = listcreate();
    //create a virus list for parent to store the bloom filters for later
    list = listcreate();
    if(parentListInit(list, input_dir, bloomsize,numMonitors) == -1) 
        exit(1);

    // prepare for select(wait for input from child)
    // to get the bloomfilters from child

    int ready_monitors = 0;


    fd_set select_fds;
    while(ready_monitors < numMonitors) {
        FD_ZERO(&select_fds);
        for(i=0; i<numMonitors; i++)
            FD_SET(sock_fds[i],&select_fds);
        int retval = select(FD_SETSIZE,&select_fds,NULL,NULL,NULL);
        sleep(0.1);
        for(i=0; i<numMonitors; i++) {
            if(FD_ISSET(sock_fds[i],&select_fds)) {
                socketbuf = pipe_read(sock_fds[i]);
                if(protocol_commands_parent(socketbuf, sock_fds, list, i, NULL, NULL) == 7)
                    ready_monitors++;
            }
        }
    }

    for(i=0; i<numMonitors; i++) {
        close(sock_fds[i]);
    } 
    
    
    int readyMonitors = 0, retval, initMonitors=0;  // counter to check if all the monitors are ready to take commands
    char *package = NULL;

    fd_set readfds; // read file descriptors for sockets
    while(1) {

        args = command_prompt();

        if(args!=NULL && args[0] != NULL && !strcmp(args[0], "/travelRequest")) {
            if(args[5]==NULL) { // if there is one less argument, then print error
                printf("Error: give more arguments\n");
                continue;
            }
            int monitor_i=numMonitors-1;
            virusListNode *vnode = get_country_node(list,&monitor_i,args[3],args[5]); // get the node of the monitor with this bloom filter for the particular virus
            if(vnode == NULL) {
                printf("Error: give valid country/virus\n");
                continue;
            }
            sock_fds[monitor_i] = create_client_socket(saddrs[monitor_i]);
            if(bloomSearch(vnode->bloom,args[1])) { // if the person is MAYBE vaccinated, look for him into monitors
                if(send_travelRequest(sock_fds, args, monitor_i) == -1)
                    exit(1);
                FD_ZERO(&readfds);
                FD_SET(sock_fds[monitor_i],&readfds);
                
                retval = select(FD_SETSIZE,&readfds,NULL,NULL,NULL);
                if(FD_ISSET(sock_fds[monitor_i],&readfds)) {                            
                    package = pipe_read(sock_fds[monitor_i]);

                    bool accepted; 
                    if( protocol_commands_parent(package, sock_fds, list, monitor_i, args,clist) == 0) 
                        accepted = 0;
                    else
                        accepted = 1;
                    
                    // now send to country: "countryTo" statistics for request
                    close(sock_fds[monitor_i]);

                    monitor_i = numMonitors-1;
                    get_country_node(list, &monitor_i,args[4], args[5]); // we just want the index "i" , to send message to the proper monitor
                    sock_fds[monitor_i] = create_client_socket(saddrs[monitor_i]);
                    send_request_response(sock_fds,list,monitor_i,args[2], args[4], args[5], accepted);
                }
            }
            else {
                printf("REQUEST REJECTED – YOU ARE NOT VACCINATED\n");
                bool accepted=0;
                add_country_global_struct(args[4]);
                add_request_global_struct(accepted,args[4]);
                if(clist_add_request(clist, accepted, args[2], args[4], args[5]) == 1) 
                    if(clist_add_request(clist, accepted, args[2], args[4], args[5]) == 1) 
                        clist_add_request(clist, accepted, args[2], args[4], args[5]);

                close(sock_fds[monitor_i]);
                monitor_i = numMonitors-1;
                get_country_node(list, &monitor_i,args[4], args[5]);
                sock_fds[monitor_i] = create_client_socket(saddrs[monitor_i]);
                send_request_response(sock_fds,list,monitor_i,args[2], args[4], args[5], accepted);
            }
        }
        else if(args!=NULL && args[0] != NULL && !strcmp(args[0], "/travelStats")) {
            if(args[3] == NULL) {
                printf("Please give more arguments!\n");
                continue;
            }
            if(get_travel_stats(args,clist) == -1) {
                printf("An error occured during sending travel stats, please try again\n");
                continue;
            }
        } 
        else if(args!=NULL && args[0] != NULL && !strcmp(args[0], "/searchVaccinationStatus")) {
            if(args[1] == NULL) {
                printf("Please give more arguments!\n");
                continue;
            }
            for(i=0; i<numMonitors; i++)
                sock_fds[i] = create_client_socket(saddrs[i]);
            if(send_search_status(sock_fds,args,numMonitors) == -1) {
                printf("An error occured during sending travel stats, please try again\n");
                continue;
            }
            for(i=0; i<numMonitors; i++)
                close(sock_fds[i]);
        
        }
        else if(args!=NULL && args[0] != NULL && !strcmp(args[0],"/addVaccinationRecords")) {
            if(args[1] == NULL) {
                printf("Please give more arguments!\n");
                continue;
            }
            int monitor_i=numMonitors-1;
            if(get_country_node_index(list, &monitor_i,args[1]) == 1) { // we just want monitor_i index,
                sock_fds[monitor_i] = create_client_socket(saddrs[monitor_i]);
                handle_addVaccinationRecords(sock_fds[monitor_i],args[1],list,monitor_i,saddrs[monitor_i]);
                close(sock_fds[monitor_i]);
            }
            else 
                printf("Please give a valid country\n");
        }
        else if(args!=NULL && args[0] != NULL && !strcmp(args[0], "/exit")) {
            if(args!=NULL)
                free_args(args); // just free the memory for args buffer
            for(i=0; i<numMonitors; i++)
                sock_fds[i] = create_client_socket(saddrs[i]);
            quit(sock_fds,numMonitors,input_dir);
            for(i=0; i<numMonitors; i++)
                close(sock_fds[i]);
            break;
        }
        if(args != NULL)
            free_args(args);
    }
    for(i=0; i<numMonitors; i++) {
        wait(NULL);

    }
    parentVirusListDestroy(list);
    parentVirusListDestroy(clist);
    delete_global_monitor();
    free(input_dir);
    free(ports);
    free(hostname);
    for(i=0; i<numMonitors; i++) 
        free(saddrs[i]);
    free(saddrs);
    //deleteFifoDir("./pipes");
    
    return 0; 
}