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


#include "list.h"
#include "global.h"
#include "msg.h"

globalData *gMonitor;


void add_global_monitor() {
    pid_t pid = getpid();

    globalData *gdata;

    gdata = calloc(1,sizeof(globalData));
    gdata->offset = 0;
    gdata->pipe_buf_size = 0;
    gdata->accepted = 0;
    gdata->requests = 0;
    gdata->rejected = 0;
    gdata->pid = pid;

    gdata->countries_size = 10;
    gdata->countries = malloc(gdata->countries_size*sizeof(char *));
    for(int i=0; i<gdata->countries_size; i++)
        gdata->countries[i] = NULL;

    gMonitor = gdata;
}
int delete_global_monitor() {
    for(int i=0; i<gMonitor->countries_size; i++) {
        if(gMonitor->countries[i]==NULL)
            break;

        free(gMonitor->countries[i]);
    }
    free(gMonitor->countries);
    free(gMonitor);
}


// add requests,accepted,rejected in global struct
void add_stats_global(Record *rec) {
    pid_t pid = getpid();

    globalData *gdata=gMonitor;

    (gdata->requests)++;
    if(rec->vaccinated) // if he is vaccinated 
        (gdata->accepted)++;
    else
        (gdata->rejected)++;
}

void add_country_global_struct(char *country) {
    int i;

    pid_t pid = getpid();

    globalData *gdata = gMonitor;

    
    char **countries = gdata->countries;

    for(i=0; i<gdata->countries_size; i++) {
        if(countries[i]==NULL)
            break;
       
        if(!strcmp(countries[i],country))    //if this country is already inside the array
            return;            // do nothing
    }
    // if we are here, it means that this country must be inserted into the array
    if(gdata->countries[gdata->countries_size-1] != NULL) {//if the array is full
        int old_size = gdata->countries_size;
        gdata->countries_size *= 2; // double the size
        char **newcountries = malloc(gdata->countries_size*sizeof(char *));
        for(i=0; i<gdata->countries_size; i++) {
            if(i < old_size) { // we want to set a pointer to the old data
                newcountries[i] = gdata->countries[i];
            }
            else if(i == old_size) { // add the new country
                newcountries[i] = calloc(strlen(country)+1, sizeof(char));
                strcpy(newcountries[i],country);
            }
            else
                newcountries[i] = NULL;
        }
        free(gdata->countries);
        gdata->countries = newcountries;
        return;
    }
    //if the array has space for another country
    // find the first NULL cell and add it
    for(i=0; i<gdata->countries_size; i++) {
        if(gdata->countries[i]==NULL ) {
            gdata->countries[i] = calloc(strlen(country)+1, sizeof(char));
            strcpy(gdata->countries[i],country);
            return;
        }
        else if(!strcmp(gdata->countries[i],country))
            return;
    }
}

int add_request_global_struct(bool accepted, char *country) {
    globalData *gdata;
    
    gdata = gMonitor;
    for(int i=0;i<gdata->countries_size; i++) {
        if(gdata->countries[i] == NULL)
            break;
        if(!strcmp(gdata->countries[i],country)) {
            (gdata->requests)++;
            if(accepted)
                (gdata->accepted)++;
            else
                (gdata->rejected)++;
            return 0;
        }
    }
    return -1;
}

int add_global_struct_request_parent(bool accepted) {
    globalData *gdata=gMonitor;

    (gdata->requests)++;
    if(accepted)
        (gdata->accepted)++;
    else
        (gdata->rejected)++;

    return 0;
}
/*
int send_country_parent() {
    pid_t pid = getpid();
    globalData *gdata;
    gdata = gMonitor;    

    char *buf = calloc(gdata->pipe_buf_size,sizeof(char));

    for(int i=0;i<gdata->countries_size; i++) {
        if(gdata->countries[i] == NULL)
            break;
        if(i==0)
            strcpy(buf,gdata->countries[i]);
        else {
            strcat(buf," ");
            strcat(buf,gdata->countries[i]);
        }  
    }
    // assign protocol 'd' to send parent country 
    if(pipe_write(gdata->socketfd,buf,strlen(buf),'d') == -1) 
        return -1;
    return 0;
}*/