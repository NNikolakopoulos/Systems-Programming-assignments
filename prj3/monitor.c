#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>

#include "hash.h"
#include "bloom.h"
#include "list.h"
#include "pipes.h"
#include "msg.h"
#include "protocol.h"
#include "functions.h"
#include "global.h"
#include "cyclic.h"
#include "logs.h"

#define perror2(s,e) fprintf(stderr, "%s: %s\n", s, strerror(e))

cyclicBuf *cb;

static pthread_mutex_t mtx;
static pthread_cond_t cond_var;

static int numPaths;

// function for threads
static void *thread_f(void *args);


// ~~~~~~~~~~~~ main ~~~~~~~~~~~~
int main(int argc, char **argv) {

    pthread_mutex_init( &mtx,NULL);
    pthread_cond_init(&cond_var,NULL);
    numPaths = 0;
    add_global_monitor();

   mtx_cond_init();
   
    HThash hash;
    virusList *list=NULL;
    virusListNode *temp;
    char *socketbuf;
    int i,j;
    unsigned int port, cyclicBufferSize, numThreads, args_path_index;
    long int bloomsize, socketBufferSize; 

    char *rest = argv[1], *token;
    while(token = strtok_r(rest," ",&rest))
    {
        if(!strcmp(token,"-p"))     
        {
            token = strtok_r(rest," ",&rest);
            port = atoi(token);
        }
        if(!strcmp(token,"-b"))     
        {
            token = strtok_r(rest," ",&rest);
            socketBufferSize = atol(token);
            socketbuf = calloc(socketBufferSize,sizeof(char));
        }
        if(!strcmp(token,"-t"))     
        {
            token = strtok_r(rest," ",&rest);
            numThreads = atoi(token);
        }
        if(!strcmp(token,"-c"))     
        {
            token = strtok_r(rest," ",&rest);
            cyclicBufferSize = atoi(token);
            cb = cyclicBuf_init(cyclicBufferSize); // initialize the cyclic buffer

        }
        if(!strcmp(token,"-s"))     
        {
            token = strtok_r(rest," ",&rest);
            bloomsize = atol(token);
            //after bloom, the paths are followed, so we must stop the loop
            break;
        }
    }   
    //set a flag in protocol.c
    set_not_is_setup();
     // set up the structures for the monitor process with these particular records
    // ( bloom filter, hash table with all the records ,.... )
    hash=HTcreate();
    list = virusListCreate();
    list->bloomsize = bloomsize;
    list->pipe_buf_size = socketBufferSize;

    // create threads
    pthread_t threads[numThreads];
    //we also need hashtable and viruslist as arguments
    threadArgs thread_args; 
    thread_args.hash = hash;
    thread_args.list = list;
    int err;
    for (i=0; i<numThreads; i++) {
        if (err = pthread_create(&threads[i], NULL, thread_f, &thread_args)) {
            printf("CREATE THREAD ERRO\n");
            perror2("pthread_create", err);
            exit(1);
        }   
    }

    bool get_input_dir_flag = true;
    // start inserting paths to cyclic buffer
    while(token = strtok_r(rest," ",&rest)) {
        //get the input directory from the 1st path
        // token =  "./input_dir./country"  
        // ^ we need input dir from token
        if(get_input_dir_flag) {
            list->input_dir = get_input_dir(token);
            get_input_dir_flag = false;
        }
        numPaths++;
        cyclicBuf_insert(cb,token);
        cond_broadcast_empty();
    }

    //wait for threads to insert the records from paths to data structures
    pthread_mutex_lock(&mtx);
    while(numPaths>0) 
        pthread_cond_wait(&cond_var,&mtx);
    pthread_mutex_unlock(&mtx);

    char *hostname = calloc(512,sizeof(char));
    if (gethostname(hostname,512) < 0) {
        perror("gethostname :");
        exit(1);
    } 

    fd_set sock_fd_set;
    int passive_sock_fd,write_sock_fd;

    struct sockaddr_in *passive_sa;
    //create a passive socket for incoming connections from parent
    passive_sa = create_sockaddr(hostname,htons(port));
    passive_sock_fd = create_server_socket(passive_sa);
    int newsock_fd = accept(passive_sock_fd,NULL,NULL);

    if(code_4(&hash, list, newsock_fd) == -1) {
        perror("error occured while sending code_4 to parent from child");
        exit(1);
    }

    set_is_setup();

    // create a node in global struct, with info about this monitor
    //add_global_monitor(writefd,readfd);

    char *package;
    int protocol_retval = -1;


    while( 1 ) {
        //for every query we will need a different connection/accept

        // if a query requires more than 1 messages to be sent to child, dont close the connection
        if(protocol_retval != 2 && protocol_retval!=4) {
            //close old file descriptor
            close(newsock_fd);
            // wait for a connection, from travelMonitor
            newsock_fd = accept(passive_sock_fd,NULL,NULL);
        }
        package = pipe_read_sig(newsock_fd);
        if(  package == NULL) {
            continue;

        }
        protocol_retval=protocol_commands_monitor(package,hash,list,newsock_fd);

        // protocol for exit
        if(protocol_retval == 9)
            break;

    }
    //for(i=0; i<numThreads; i++)  
        //pthread_join(threads[i], NULL);
    mtx_cond_destroy(numThreads);
    for(i=0; i<numThreads; i++)  
        pthread_join(threads[i], NULL);
    cyclicBuf_delete(cb);
    //create log file
    create_logs(hash,list->pipe_buf_size);
    HTdestroy(hash);
    virusListDestroy(list);
    free(passive_sa);
    free(socketbuf);
    free(hostname);
    delete_global_monitor();
    close(passive_sock_fd);
}

static int count=0;
static void *thread_f(void *args) {
    char *path;
    threadArgs *thread_args = (threadArgs *)args;
    virusList *vlist = thread_args->list;
    while(1){
        path = cyclicBuf_pop(cb);
        cond_signal_full();
        if(path == NULL)
            break;
        pthread_mutex_lock(&mtx);
        (thread_args->hash) = HTinsertfiles((thread_args->hash),vlist,path);
        add_country_global_struct(path);
        // one less path for monitor to read
        numPaths--;
        // if there are no more paths, inform parent that all threads are done with inserting new records
        if(numPaths == 0)
            pthread_cond_signal(&cond_var);
        pthread_mutex_unlock(&mtx);

    }
    return NULL;
}