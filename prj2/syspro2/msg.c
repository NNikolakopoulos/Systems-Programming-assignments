#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>

#include "list.h"
#include "protocol.h"
#include "global.h"
#include "functions.h"
#include "pipes.h"

extern globalData *gMonitor;


int safe_read(int fd,char* buf,size_t size, bool signals)
{
    char *tempbuf=buf;
    size_t len=0;
    int n;
    while( len < size )    // while not exceeding size of bytes must be read
    {   

        n = read( fd, tempbuf, size-len);
        if( n == 0)     
            return 0;                      
        if(n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            if(  errno == EINTR && signals)  // if a signal occured while read was in progress and signals flag is true
                continue;                 //try reading again
            if( errno != EINTR && !signals )
                perror("safe read failed");              // if an other error occured or signal interrupt with signals flag=false 
            return -1;
        }
        
        len=len+n;          // by increasing this len, it means that we are decreasing the size of bytes we want to read
        tempbuf=tempbuf+n;  //but we must continue 
    }
    return len;  //number of bytes read
}

int safe_write(int fd,char* buf,size_t size)
{

    char *tempbuf=buf;
    size_t len=0;
    int n;
    while( len < size )
    {
        if( (n = write( fd, tempbuf, size-len)) >= 0 )
        {
            len=len+n;    
            tempbuf=tempbuf+n;
        }
        else if( errno == EINTR )    //if interrupted by signal
            continue;               //continue
        else
            return -1;               //if error occured, return bytes writeen

    }
    return len;   //bytes written
}

int pipe_write(int fd, char *buf,long int bufsiz, char protocol) {
    char *newbuf = malloc( (bufsiz + 11) * sizeof(char));

    // every pipe write should have the same type:
    // protocol(1 byte) + length_of_message(at most 9 bytes) + message
    // dont forget the offset for the following snprintf's.

    newbuf[0]=protocol;
    snprintf(newbuf+1, 10, "%09ld",bufsiz);
    snprintf(newbuf+10,bufsiz+1,"%s",buf);

    if(safe_write(fd,newbuf,bufsiz+10) == -1) {
        perror("Write for pipe failed");
        free(newbuf);
        return -1;
    }
    free(newbuf);
}

char get_protocol(char *buf) {
    return buf[0];
}

long int get_package_length(char *buf) {
    char *lengthstr = calloc(10,sizeof(char));
    strncpy(lengthstr,buf+1,9);
    long int length = atoi(lengthstr);
    free(lengthstr);
    return length;
}

char *get_msg(char *buf) {
    return buf+10; // skip the protocol + length_of_buf (= 10 bytes)
}

char *pipe_read(int fd) {
    char *protocol_bufsize = calloc(11,sizeof(char));

    // read protocol + package size
    if(safe_read(fd,protocol_bufsize,10,1) == -1) {
        perror("Error while reading from pipe");
        return NULL;
    }
    protocol_bufsize[10] = '\0';
    int package_size = atoi(protocol_bufsize+1);
    char *package = calloc((package_size + 11) ,sizeof(char)); // allocate memory in heap for the protocol+package_size+package

    strcpy(package,protocol_bufsize);
    if(safe_read(fd,package+10,package_size,1) == -1) {
        perror("Error while reading from pipe");
        return NULL;
    }
    //printf("%d\n",package_size);
    package[package_size + 10 ] = '\0';
    free(protocol_bufsize);
    return package;

}


char *pipe_read_sig(int fd) {
    char *protocol_bufsize = calloc(11,sizeof(char));

    // read protocol + package size
    if(safe_read(fd,protocol_bufsize,10,0) == -1) {
        if(errno != EINTR)
            perror("Error while reading from pipe");
        return NULL;
    }
    protocol_bufsize[10] = '\0';
    int package_size = atoi(protocol_bufsize+1);
    char *package = calloc((package_size + 11) ,sizeof(char)); // allocate memory in heap for the protocol+package_size+package

    strcpy(package,protocol_bufsize);
    if(safe_read(fd,package+10,package_size,0) == -1) {
        if(errno != EINTR)
            perror("Error while reading from pipe");
        return NULL;
    }
    //printf("%d\n",package_size);
    package[package_size + 10 ] = '\0';
    free(protocol_bufsize);
    return package;

}


int initialize_monitors0(int *fds, unsigned long bloomsize, unsigned long bufsize, char *input_dir, int numMonitors) {
    char *package;
    int i;

    package = malloc(bufsize*sizeof(char));
    snprintf(package,50,"%ld %ld %s",bloomsize,bufsize,input_dir);

    for(i=0; i<numMonitors; i++) {
        //assign protocol 1 for initialization ( bloomsize , pipe uiffer size and input directory to monitors)
        if(pipe_write(fds[2*i+1],package,strlen(package),'1') == -1) 
            return -1;
    }
    free(package);
}

int initialize_monitors(int *fds, char *input_dir, int numMonitors) {

    DIR *dirp;
    struct dirent *direntp;
    char **packages, *temp, *package = malloc(30*sizeof(char));
    int i;

    if((dirp=opendir(input_dir)) == NULL) {
        fprintf(stderr,"cannot open %s\n",input_dir);
        return -1;
    }   
    /*
    packages = (char **) malloc(numMonitors*sizeof(char *));
    
    for(i=0; i<numMonitors; i++) {
        packages[i] = calloc(30,sizeof(char));
        snprintf(packages[i],30,"%ld %ld %s",bloomsize,bufsize,input_dir);
    }
    */

    i = 0;  // index for i-th monitor
    fd_set writefds;
    while ((direntp = readdir(dirp)) != NULL)
    {
        if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, "..") || direntp->d_ino == 0)  // Ignore . and .. dirs
            continue;
        char *dentry = direntp->d_name;
        snprintf(package, strlen(dentry)+1, "%s",dentry);
        // assign protocol 2 for sending countries to monitors
        if(pipe_write(fds[2*i+1],package,strlen(package),'2') == -1) 
            return -1;

        i = (i+1)%numMonitors; // round robin split of countries
    }
    free(package);
    closedir(dirp);
    return 0;
    /*
    i = 0;  // index for i-th monitor
    while ((direntp = readdir(dirp)) != NULL)
    {
        if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, "..") || direntp->d_ino == 0)  // Ignore . and .. dirs
            continue;

        int package_size = strlen(packages[i]), newCountry_size = (strlen(direntp->d_name) + 1);
        if( ( package_size + newCountry_size ) >= package_size) {
            temp = malloc((package_size + newCountry_size) *sizeof(char));
            strcpy(temp,packages[i]);
            free(packages[i]);
            packages[i] = temp;
            //packages[i] = realloc(packages[i], package_size + newCountry_size );
        }
        snprintf(packages[i]+package_size, newCountry_size+1, " %s",direntp->d_name);

        i = (i+1)%numMonitors; // round robin split of countries
    }

    for(i=0; i<numMonitors; i++) {
        //assign protocol 0 for initialization
        if(pipe_write(fds[2*i+1],packages[i],strlen(packages[i]),'0') == -1) 
            return -1;
    } */

    
    /*
    for (int i = 0; i < num_workers; ++i)  // Send End of Task / Availability check
        send_message(w_stats[i].writ_fd, AVAILABILITY_CHECK, "", buf_size); */
}


int parentListInit(List *list, char *input_dir, unsigned long bloomsize, int numMonitors) {
    DIR *dirp;
    struct dirent *direntp;
    virusListNode *node;
    char **countries = NULL, *temp = NULL;
    int i;

    if((dirp=opendir(input_dir)) == NULL) {
        fprintf(stderr,"cannot open %s\n",input_dir);
        return -1;
    }   

    countries = (char **) malloc(numMonitors*sizeof(char *));
    
    for(i=0; i<numMonitors; i++)
        countries[i] = calloc(30,sizeof(char));

    i = 0;  // index for i-th monitor

    while ((direntp = readdir(dirp)) != NULL)
    {
        if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, "..") || direntp->d_ino == 0)  // Ignore . and .. dirs
            continue;

        int old_countries_size = (countries[i]==NULL) ? 0 : strlen(countries[i]), newCountry_size = (strlen(direntp->d_name) + 1);
        if( ( old_countries_size + newCountry_size ) >= old_countries_size) {
            temp = calloc((old_countries_size + newCountry_size + 1) ,sizeof(char));
            strcpy(temp,countries[i]);
            free(countries[i]);
            countries[i] = temp;
        }        
        snprintf(countries[i]+old_countries_size, newCountry_size+1, " %s",direntp->d_name);
        i = (i+1)%numMonitors; // round robin split of countries
    }

    Listnode *newnode;
    virusParentNode *vpnode;

    // create a list, with data= virusParentNode(countries_name, virusList -> bloom filter for each virus) 

    for( i=0; i<numMonitors; i++) {
        newnode = malloc(1*sizeof(Listnode));
        vpnode = malloc(1*sizeof(virusParentNode));
        newnode -> item = vpnode;
        newnode -> next = list->head;
        list->head = newnode; 
        (list->size)++;

        vpnode->country = countries[i];
        vpnode->vlist = virusListCreate();
        vpnode->vlist->bloomsize=bloomsize;
    }
    closedir(dirp);
    free(countries);

    return 0;
}


int ready_for_bloom(int *fds, int numMonitors) {
    int i;
    //char *package = calloc(12,sizeof(char));
    
    for(i=0; i<numMonitors; i++) {
        if(pipe_write(fds[2*i+1],"",1,'4') == -1)  // assign protocol '4' to tell monitors that parent is ready to get bloom filters
            return -1;
    }
}

int ready_to_exit(int *fds, int numMonitors) {
    int i;
    
    for(i=0; i<numMonitors; i++) {
        if(pipe_write(fds[2*i+1],"",1,'9') == -1)  // assign protocol '9' to tell monitors to exit
            return -1;
    }
}

int quit(pid_t *pids,int numMonitors) {
    for (int i = 0; i < numMonitors; i++)  // Kill all monitors
        kill(pids[i], SIGKILL);
  
  for (int i = 0; i < numMonitors; i++)  // Wait for monitors to terminate
        wait(NULL);

    char *buf = calloc(100,sizeof(char));
    char *path = calloc(30, sizeof(char));

    snprintf(path,30,"./logs/log.%d",getpid());
    int fd = open(path,O_CREAT|O_WRONLY|O_TRUNC|O_APPEND,0777);

    globalData *gdata = gMonitor;
    for(int i=0; i<gdata->countries_size; i++) {
        if(gdata->countries[i] == NULL) 
            break;
        
        unsigned long buf_length = strlen(gdata->countries[i]) + 2;
        snprintf(buf,buf_length,"%s\n",gdata->countries[i]);
        if(safe_write(fd,buf,buf_length) == -1) {
            perror("write in initialization of logs failed");
            exit(1);
        }
    }

    snprintf(buf,100,"\nTOTAL TRAVEL REQUESTS %lu\nACCEPTED %lu\nREJECTED %lu\n",gdata->requests,gdata->accepted,gdata->rejected);
    if(safe_write(fd,buf,strlen(buf)) == -1) {
        perror("write in initialization of logs failed");
        exit(1);
    }
    close(fd);
    free(buf);
    free(path);
    

}

int send_travelRequest(int *fds,char **args, int monitor_i) {
    char *pipe_buf;
    // we will need memory for citizenid , date, countryFrom, virus name , for 2 space characters and '\0'
    unsigned long pipe_buf_size = strlen(args[1])+strlen(args[2])+strlen(args[3])+strlen(args[5])+4;

    pipe_buf = malloc(pipe_buf_size *sizeof(char));

    snprintf(pipe_buf,pipe_buf_size,"%s %s %s %s",args[1],args[2],args[3],args[5]);
    if(pipe_write(fds[2*monitor_i+1],pipe_buf,pipe_buf_size,'5') == -1)  // assign protocol '5' to send monitors travel request
            return -1;
    
    free(pipe_buf);
    return 0;
}

int send_request_response(int *fds, List *list, int numMonitors, char *date, char *country, char *virusname, bool accepted) {
    char *pipe_buf;

    pipe_buf = calloc(100, sizeof(char));

    snprintf(pipe_buf,100,"%s %s %d",date,country,accepted);

    int monitor_i = numMonitors-1;
    get_country_node(list, &monitor_i,country, virusname); // we just want the index "i" , to send message to the proper monitor

    if(pipe_write(fds[2*monitor_i+1],pipe_buf,strlen(pipe_buf),'a') == -1)  // assign protocol 'a' to send travel request stats to parent
        return -1;
    
    free(pipe_buf);
}

int get_travel_stats(char **args, List *list) {

    char *results;
    int i;
    unsigned long request = 0, accepted = 0, rejected =0;

    char *reversedDate1 = strdup(args[2]), *reversedDate2 = strdup(args[3]);
    reversedDate1=reverse_date(reversedDate1);
    reversedDate2=reverse_date(reversedDate2);


    if(args[4]!=NULL) { // if country argument is given

        virusListNode *vnode = get_vnode(list,args[4], args[1]);
        if(vnode != NULL) {
            Listnode *listnode = vnode->req_stats->head;
            while(listnode!=NULL) {
                req_stat *request_statistics = (req_stat *) listnode->item;
                request++;
                if(strcmp(request_statistics->date,reversedDate1)>0 &&  strcmp(request_statistics->date,reversedDate2)<0 && request_statistics->accepted)
                    accepted++;
                else
                    rejected++;
                listnode = listnode->next;
            }
            printf("Total Requests: %ld\nAccepted: %ld\nRejected: %ld\n",request,accepted,rejected);
        }

    }
    else {  
        results = get_stats_all_countries(list,reversedDate1,reversedDate2,args[1]);
        if(results!=NULL) {
            printf("%s\n",results);
            free(results);
        }
        else
            printf("No results\n");
        
    }

    free(reversedDate1);
    free(reversedDate2);
} 

// this function sends message to all monitors , to search for this citizenID
int send_search_status(int *pipesfds,char **args, int numMonitors) {
    int i;
    unsigned long pipe_buf_length = (strlen(args[1]) + 1);
    char *pipe_buf = calloc(pipe_buf_length,sizeof(char));

    strcpy(pipe_buf,args[1]); // citizenID

    for(i=0; i<numMonitors; i++) {
        if(pipe_write(pipesfds[2*i+1],pipe_buf,pipe_buf_length,'b') == -1)  // assign protocol 'b', for this job
            return -1;
    }
    fd_set readfds;
    int countMonitors=0;
    while(1) {
        FD_ZERO(&readfds);
        for(i=0; i<numMonitors; i++) 
            FD_SET(pipesfds[2*i],&readfds);
        
        int retval = select(FD_SETSIZE,&readfds,NULL,NULL,NULL);
        for(i=0; i<numMonitors; i++) {
            if(FD_ISSET(pipesfds[2*i],&readfds)) {                            
                pipe_buf = pipe_read(pipesfds[2*i]);
                retval = protocol_commands_parent(pipe_buf, pipesfds, NULL, 0, args,NULL);
                if( retval == 7)
                    break;
                else if( retval == 10)
                    countMonitors++;
            }
        }
        if(retval == 7)
            break;
        else if(countMonitors == numMonitors) {
            printf("Didn't found the citizen\n");
            break;
        }
    }
}


int spawn_monitor( int *pipesfds, List *list,pid_t sig_chld_pid, char *input_dir, int numMonitors, pid_t *pids, unsigned long bloomsize, unsigned long pipe_buf_size) {
    DIR *dirp;
    struct dirent *direntp;
    char *package = calloc(100,sizeof(char));
    int i, sig_chld_i;

    for (sig_chld_i=0; sig_chld_i<numMonitors; sig_chld_i++) {
        if(pids[sig_chld_i] == sig_chld_pid)
            break;
    }

    //close fifo read end and write end of parent, and then delete them
    close(pipesfds[2*sig_chld_i]);
    close(pipesfds[2*sig_chld_i+1]);

    char *path = calloc(30,sizeof(char));
    snprintf(path,30,"./pipes/fifo_%d_R",sig_chld_i);
    if( remove(path) == -1) {
        perror("remove error");
        return -1;
    }
    snprintf(path,30,"./pipes/fifo_%d_W",sig_chld_i);
    if( remove(path) == -1) {
        perror("remove error");
        return -1;
    }
    free(path);

    //create a new fifo
    char *pipeW = calloc(30,sizeof(char)), *pipeR = calloc(30,sizeof(char));

    createFifo("./pipes",pipeR,pipeW,sig_chld_i);
    if (( pipesfds[2*sig_chld_i+1] = open ( pipeW ,  O_RDWR   | O_NONBLOCK )) < 0)
    { 
        perror ( " fifo open error " ); 
        exit(1);
    }
    if (( pipesfds[2*sig_chld_i] = open ( pipeR , O_RDONLY  | O_NONBLOCK )) < 0)
    { 
        perror ( " fifo open error " ); 
        exit(1);
    }

    pids[sig_chld_i] = fork();
    if(pids[sig_chld_i] == 0)  //if its a child
        execlp("monitor","./monitor",pipeR,pipeW,(char *) NULL);

    int flags;
    /* clear O_NONBLOCK  and reset file flags                 */
    flags = O_WRONLY;
    if ((fcntl(pipesfds[2*sig_chld_i+1],F_SETFL,flags)) == -1) {
        printf("fcntl returned -1\n" );
        exit(4);
    }
    /* clear O_NONBLOCK  and reset file flags                 */
    flags = O_RDONLY;
    if ((fcntl(pipesfds[2*sig_chld_i],F_SETFL,flags)) == -1) {
        printf("fcntl returned -1\n" );
        exit(4);
    }
    
    //send basic information to the child, like pipe_buffer_size, etc.
    snprintf(package,100,"%ld %ld %s",bloomsize,pipe_buf_size,input_dir);

    //assign protocol 1 for initialization ( bloomsize , pipe uiffer size and input directory to monitors)
    if(pipe_write(pipesfds[2*sig_chld_i+1],package,strlen(package),'1') == -1) 
        return -1;

    // send all countries to child
    if((dirp=opendir(input_dir)) == NULL) {
        fprintf(stderr,"cannot open %s\n",input_dir);
        return -1;
    }   
    
    i = 0;  // index for i-th monitor

    while ((direntp = readdir(dirp)) != NULL)
    {
        if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, "..") || direntp->d_ino == 0)  // Ignore . and .. dirs
            continue;
        if(sig_chld_i == i) { // if its the child that got terminated, send it the countries
            char *dentry = direntp->d_name;
            snprintf(package, strlen(dentry)+1, "%s",dentry);
            // assign protocol 2 for sending countries to monitors
            if(pipe_write(pipesfds[2*i+1],package,strlen(package),'2') == -1) 
                return -1;
        }
        i = (i+1)%numMonitors; // round robin split of countries
    }

    if(pipe_write(pipesfds[2*sig_chld_i+1],"",0,'4') == -1) { // tell monitors that parent is ready to get bloom filters
        perror("error while sending ready for bloom to monitor for handling usr1");
        exit(1);
    }
    while(1) {
        char *package;
        package = pipe_read(pipesfds[2*sig_chld_i]);
        if(package == NULL)
            continue;
        if(protocol_commands_parent(package,NULL,list,sig_chld_i,NULL,NULL) == 7) 
            break;
    }


}