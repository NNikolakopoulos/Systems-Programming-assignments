#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>
#include <string.h>

#include "hash.h"
#include "bloom.h"
#include "list.h"
#include "pipes.h"
#include "msg.h"
#include "protocol.h"
#include "functions.h"
#include "logs.h"
#include "signals_parent.h"
#include "global.h"

int sig_updates;

int main(int argc, char **argv) {

    signals_init();
    sig_block();
    long int bloomsize = 100000, bufsize = 1024;
    int numMonitors = 2;
    char *input_dir=calloc(30,sizeof(char));



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
            bufsize = atol(argv[i+1]);
        }
        if(!strcmp(argv[i],"-i"))     
        {
            strcpy(input_dir,argv[i+1]);
        }
    }

    int pipesfds[2*numMonitors],i;
    pid_t pid[numMonitors];
    char *pipeR=calloc(20,sizeof(char)), *pipeW=calloc(20,sizeof(char)), **args;
    bool updateDatabase = false, init_monitors = true, ready_for_bloomfilters = false;
    List *list, *clist;
    pid_t sig_chld_pid; // pid of monitor that got killed unexpected

    add_global_monitor(0,0); // initialize global struct for parent, to save stats for later use
    handle_signals(0, &sig_updates, &sig_chld_pid); // initialize sig_updates global variable
    delete_log_dir();
    createFifoDir("./pipes");
    for( i=0; i<numMonitors; i++) {
        //create fifo for ipc with parent-child
        createFifo("./pipes",pipeR,pipeW,i);
        //open fifo's and save the fds in an array
        if (( pipesfds[2*i+1] = open ( pipeW ,  O_RDWR   | O_NONBLOCK )) < 0)
        { 
            perror ( " fifo open error " ); 
            exit(1);
        }
        if (( pipesfds[2*i] = open ( pipeR , O_RDONLY  | O_NONBLOCK )) < 0)
        { 
            perror ( " fifo open error " ); 
            exit(1);
        }

        pid[i] = fork();
        if(pid[i] == 0)  //if its a child
            execlp("monitor","./monitor",pipeR,pipeW,(char *) NULL);

        int flags;

        /* clear O_NONBLOCK  and reset file flags                 */
        flags = O_WRONLY;
        if ((fcntl(pipesfds[2*i+1],F_SETFL,flags)) == -1) {
            printf("fcntl returned -1\n" );
            exit(4);
        }


        /* clear O_NONBLOCK  and reset file flags                 */
        flags = O_RDONLY;
        if ((fcntl(pipesfds[2*i],F_SETFL,flags)) == -1) {
            printf("fcntl returned -1\n" );
            exit(4);
        }

    }

    // create a list for storing the requests
    clist = listcreate();

    //create a virus list for parent to store the bloom filters for later
    list = listcreate();
    if(parentListInit(list, input_dir, bloomsize,numMonitors) == -1) 
        exit(1);
    

    //initialize monitors: give bloomsize, buff size for pipe and countries for each one
    if( initialize_monitors0(pipesfds, bloomsize, bufsize, input_dir, numMonitors) == -1) {
        perror("Error with initialization of monitors");
        exit(1);
    }
    int readyMonitors = 0, retval, initMonitors=0;  // counter to check if all the monitors are ready to take commands
    char *package = NULL;

    fd_set readfds; // read file descriptors for pipe
    while(1) {

        if(init_monitors) {
            FD_ZERO(&readfds);
            for(i=0; i<numMonitors; i++) 
                FD_SET(pipesfds[2*i],&readfds);
            
            retval = select(FD_SETSIZE,&readfds,NULL,NULL,NULL);
            for(i=0; i<numMonitors; i++) {
                if(FD_ISSET(pipesfds[2*i],&readfds)) {
                    package = pipe_read(pipesfds[2*i]);
                    if(protocol_commands_parent(package, pipesfds, list, i, NULL, clist) == 0)
                        initMonitors++;

                }
            }
            if(initMonitors == numMonitors) {
                initialize_monitors(pipesfds, input_dir, numMonitors);
                init_monitors = false;
                updateDatabase = false;
                ready_for_bloomfilters = true;
            }
        }
        if(ready_for_bloomfilters) {
            if(ready_for_bloom(pipesfds,numMonitors) == -1)
                exit(1); 
            updateDatabase = true;
            ready_for_bloomfilters = false;
        }
        if( updateDatabase ) {
            FD_ZERO(&readfds);
            for(i=0; i<numMonitors; i++) 
                FD_SET(pipesfds[2*i],&readfds);
            
            retval = select(FD_SETSIZE,&readfds,NULL,NULL,NULL);
            for(i=0; i<numMonitors; i++) {
                if(FD_ISSET(pipesfds[2*i],&readfds)) {
                    package = pipe_read(pipesfds[2*i]);
                    if (protocol_commands_parent(package, pipesfds, list, i, NULL, clist) == 7)
                        readyMonitors++;
                }
            }
            if(numMonitors == readyMonitors)
                updateDatabase = false;
        }
        else if(!updateDatabase && !ready_for_bloomfilters && !init_monitors){

            sig_unblock();
            sig_block();
            sig_got();
            sig_unblock();

            args = command_prompt();

            if(args == NULL) {
                sig_block();
                sig_got();
            }

            if(sig_updates == 1) { // monitor sent parent usr2, and now sent back to monitor countries to insert again
                // an
                FD_ZERO(&readfds);
                for(i=0; i<numMonitors; i++) 
                    FD_SET(pipesfds[2*i],&readfds);
                
                retval = select(FD_SETSIZE,&readfds,NULL,NULL,NULL);
                for(i=0; i<numMonitors; i++) {
                    if(FD_ISSET(pipesfds[2*i],&readfds)) {                            
                        package = pipe_read(pipesfds[2*i]);
                        protocol_commands_parent(package, pipesfds, list, i, NULL,NULL);
                    }
                }
                sig_updates=0;
            }
            else if(sig_updates == 3) { // got SIG_CHLD signal, so spawn a new monitor, to replace the old one
                spawn_monitor(pipesfds, list, sig_chld_pid, input_dir, numMonitors, pid, bloomsize, bufsize);
                sig_updates=0;
            }
            else if(args!=NULL && args[0] != NULL && !strcmp(args[0], "/travelRequest")) {
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
                if(bloomSearch(vnode->bloom,args[1])) { // if the person is MAYBE vaccinated, look for him into monitors
                    if(send_travelRequest(pipesfds, args, monitor_i) == -1)
                        exit(1);
                    FD_ZERO(&readfds);
                    for(i=0; i<numMonitors; i++) 
                        FD_SET(pipesfds[2*i],&readfds);
                    
                    retval = select(FD_SETSIZE,&readfds,NULL,NULL,NULL);
                    for(i=0; i<numMonitors; i++) {
                        if(FD_ISSET(pipesfds[2*i],&readfds)) {                            
                            package = pipe_read(pipesfds[2*i]);

                            bool accepted; 
                            if( protocol_commands_parent(package, pipesfds, list, i, args,clist) == 0) 
                                accepted = 0;
                            else
                                accepted = 1;
                            send_request_response(pipesfds,list,numMonitors,args[2], args[4], args[5], accepted);
                        }
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
                    send_request_response(pipesfds,list,numMonitors,args[2], args[4], args[5], accepted);
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
                if(send_search_status(pipesfds,args,numMonitors) == -1) {
                    printf("An error occured during sending travel stats, please try again\n");
                    continue;
                }

            
            }
            else if(args!=NULL && args[0] != NULL && !strcmp(args[0],"/addVaccinationRecords")) {
                 if(args[1] == NULL) {
                    printf("Please give more arguments!\n");
                    continue;
                }
                int monitor_i=numMonitors-1;
                if(get_country_node_index(list, &monitor_i,args[1]) == 1) { // we just want monitor_i index,
                    if (kill(pid[monitor_i], SIGUSR1) == -1) {
                        perror("Error sending usr1 to child, for /addVaccinationRecords function!"); 
                        exit(1);
                    }
                }
                else 
                    printf("Please give a valid country\n");
            }
            else if(sig_updates == 2 || (args!=NULL && args[0] != NULL && !strcmp(args[0], "/exit"))) {
                if(args!=NULL)
                    free_args(args); // just free the memory for args buffer
                quit(pid,numMonitors);
                //ready_to_exit(pipesfds,numMonitors);
                free(pipeR);
                free(pipeW);
                break;
            }
            if(args != NULL)
                free_args(args);
        }
    }
    for(i=0; i<numMonitors; i++) {
        close(pipesfds[2*i]);
        close(pipesfds[2*i+1]);
    }
    parentVirusListDestroy(list);
    parentVirusListDestroy(clist);
    delete_global_monitor();
    free(input_dir);
    //deleteFifoDir("./pipes");
    
    return 0; 
}