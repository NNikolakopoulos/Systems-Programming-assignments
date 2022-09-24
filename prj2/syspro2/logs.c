
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>
#include <dirent.h>

#include "hash.h"
#include "msg.h"
#include "global.h"

extern globalData *gMonitor;

void delete_log_dir() {
    DIR *dirp;
    struct dirent *direntp;
    char log_dir[] = "./logs";

    if((dirp=opendir(log_dir)) == NULL) {
        if(errno == ENOENT) {
            if(mkdir(log_dir,0777) == -1) {
                perror("cannot create log directory");
                exit(1);
            }
            return;

        }
        else
            fprintf(stderr,"cannot open %s\n",log_dir);
        exit(1);
    } 

    char *logfilepath = calloc(30,sizeof(char));
    while ((direntp = readdir(dirp)) != NULL)
    {
        if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, "..") || direntp->d_ino == 0)  // Ignore . and .. dirs
            continue;
        char *dentry = direntp->d_name;
        snprintf(logfilepath,30,"%s/%s",log_dir,dentry);
        if(remove(logfilepath) == -1) {
            perror("remove error while deleting log files");
            exit(1);
        }
    }
    free(logfilepath);
    closedir(dirp);
}

void create_logs(HThash hash, unsigned long pipe_buf_size) {

    int fd;
    pid_t pid = getpid();
    char *buf = calloc(25,sizeof(char));
    long int buf_length;

    char *path = calloc(30, sizeof(char));
    snprintf(path,30,"./logs/log.%d",pid);
    fd = open(path,O_CREAT|O_WRONLY|O_TRUNC,0777);

    globalData *gdata=gMonitor;

    gdata->pipe_buf_size = pipe_buf_size;

    countryNode *cnode;

    Listnode *listnode = hash.countryList->head;
    while(listnode!=NULL) {
        cnode  = (countryNode *) listnode->item;
        // compute the length of country name + '\n' + '\0'
        buf_length = strlen(cnode->country) + 2;
        snprintf(buf,buf_length,"%s\n",cnode->country);
        if(safe_write(fd,buf,buf_length) == -1) {
            perror("write in initialization of logs failed");
            exit(1);
        }
        buf_length -= 1; // now substract the byte for '\0'
        gdata->offset += buf_length; // add the length of country name + '\n' , to offset

        listnode = listnode->next;
    }

    free(buf);
    free(path);
    return;
}

void write_in_log() {
    pid_t pid = getpid();

    globalData *gdata = gMonitor;

    if(gdata == NULL) { // this will never happen, but just in case...
        printf("Make sure to save info about this monitor in global structs, this signal will be skipped");
        return;
    }

    char *buf = calloc(256,sizeof(char));
    snprintf(buf,256,"TOTAL TRAVEL REQUESTS %lu\nACCEPTED %lu\nREJECTED %lu\n",gdata->requests,gdata->accepted,gdata->rejected);

    char *path = calloc(20,sizeof(char));
    snprintf(path,20,"./logs/log.%d",pid);
    //printf("%s %s\n",buf,path);
    /*
    FILE* fp = fopen(path, "w");
    fseek( fp, gdata->offset-1, SEEK_SET );
    fputs(buf, fp); */
    int fd = open(path,O_WRONLY|O_APPEND,0777);
    if(safe_write(fd,buf,strlen(buf)) == -1) {
        perror("write in initialization of logs failed");
        exit(1);
    }

    free(buf);
    free(path);
    close(fd);
}


