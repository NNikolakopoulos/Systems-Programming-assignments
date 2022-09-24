#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>

void deleteFifoDir(char *path);

void createFifoDir(char *path) {
        int retval = mkdir(path, 0777);//S_IRWXU | S_IRGRP | S_IROTH);
        if( retval == -1 &&  EEXIST == errno) {
           /*fprintf(stderr,"ERROR: directory \"%s\" exists, cannot create a direcotry for fifo's, exiting...\n",path);
            exit(10); */
            deleteFifoDir(path); //if it exists, delete it
            createFifoDir(path);
        }
        else if(retval==-1) {
            perror("An error occured making directory for fifo's\n");
            exit(11);
        }     
}

// given the path of the fifo directory and allocated buffers for pipe read and write(refers to parent)
void createFifo(char *path, char *pipeR, char *pipeW,int i) {
    snprintf(pipeR,20,"%s/fifo_%d_R",path,i);
    if( mkfifo(pipeR,0777) == -1 ) {  //S_IRWXU | S_IRGRP | S_IROTH)
        perror (" mkfifo failed " ) ;
        exit(12) ;
    }
    snprintf(pipeW,20,"%s/fifo_%d_W",path,i);
    if( mkfifo(pipeW,0777) == -1 ) {  //S_IRWXU | S_IRGRP | S_IROTH)
        perror (" mkfifo failed " ) ;
        exit(12) ;
    }
}

void deleteFifoDir(char *path) {
    DIR *dirp, *tempdir;
    struct dirent *direntp;
    char *newpath = NULL;

    if((dirp=opendir(path)) == NULL) {
        fprintf(stderr,"cannot open %s\n",path);
        exit(1);
    }   
    
    while( (direntp = readdir(dirp)) != NULL) {
        // skip current dir and parent dir. 0 inode number means an error occured, so try again
        if(direntp->d_ino == 0 || !strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, "..")) 
            continue;

        newpath = (char *) malloc( strlen(path) + strlen(direntp->d_name) + 3 );
        strcpy(newpath,path);
        strcat(newpath,"/");
        strcat(newpath,direntp->d_name);

        if( remove(newpath) == -1) {
            perror("Error: cannot delete pipe");
            exit(13);
        }
        free(newpath);
    } 
    rmdir(path);
    closedir(dirp);
}





