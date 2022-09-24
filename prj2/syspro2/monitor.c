#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>

#include "hash.h"
#include "bloom.h"
#include "list.h"
#include "pipes.h"
#include "msg.h"
#include "protocol.h"
#include "functions.h"
#include "signals_child.h"
#include "global.h"

int main(int argc, char **argv) {

    signals_init();
    sig_block();
   
    HThash hash;
    virusList *list=NULL;
    virusListNode *temp;
    char dir[]="./input_dir/Albania", *pipebuf=calloc(1024,sizeof(char)), *fifoW=strdup(argv[1]), *fifoR=strdup(argv[2]);

    fd_set writefds, readfds;
    int writefd, readfd;

    if(argc != 3) {
        printf("Error: provide a pipe for monitor\n");
        exit(20);
    }

    // set up the structures for the monitor process with these particular records
    // ( bloom filter, hash table with all the records ,.... )
    hash=HTcreate();
    list = virusListCreate();


    //set up the named pipes for IPC
    if (( writefd = open ( fifoW , O_RDWR  | O_NONBLOCK )) < 0)
    { 
        perror ( " fifo open error " ); 
        exit(1);
    }
    if (( readfd = open ( fifoR ,  O_RDONLY | O_NONBLOCK )) < 0)
    { 
        perror ( " fifo open error " ); 
        exit(1);
    }

    // create a node in global struct, with info about this monitor
    add_global_monitor(writefd,readfd);


    int flags;

    /* clear O_NONBLOCK  and reset file flags                 */
    flags = O_WRONLY;
    if ((fcntl(writefd,F_SETFL,flags)) == -1) {
        printf("fcntl returned -1\n" );
        exit(4);
    }

    /* clear O_NONBLOCK  and reset file flags                 */
    flags = O_RDONLY;
    if ((fcntl(readfd,F_SETFL,flags)) == -1) {
        printf("fcntl returned -1\n" );
        exit(4);
    }

    char *package;

    fd_set readfd_set;
    int select_retval, protocol_retval;

    sig_unblock();

    while( 1 ) {
        sig_block();
        sig_got();
        sig_unblock();

        package = pipe_read_sig(readfd);

        sig_block();

        if(  package == NULL) {
            continue;

        }
        protocol_retval=protocol_commands_monitor(package,hash,list,writefd);
        if(protocol_retval == 9)
            break;

    }

    close(readfd);
    close(writefd);
    HTdestroy(hash);
    virusListDestroy(list);
    free(pipebuf);
    free(fifoR);
    free(fifoW);
}
