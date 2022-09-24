#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include "hash.h"
#include "bloom.h"
#include "list.h"
#include "global.h"
#include "protocol.h"



char **command_prompt() {
    char **args, *buf, *rest, *token;
    size_t len = 200;
    int i;

    buf = (char *) calloc(len,sizeof(char)); // create a buffer to store temporary the input arguments
    args = (char **) malloc(6*sizeof(char *)); // return a 2d array with the arguments
    for(i=0; i<6; i++) 
        args[i] = NULL;

    printf("Please insert command:\n");
    if(fgets(buf,len,stdin) ==NULL) {
        //perror("getline error");
        free(buf);
        free(args);
        return NULL;
    }

    rest = buf;
    i=0;
    while(token = strtok_r(rest," ",&rest)) {
        args[i] = malloc( (strlen(token)+1) *sizeof(char));
        strcpy(args[i],token);
        if(i==5) 
            args[i][strlen(token)-1] = '\0';
        i++;
    }
    free(buf);
    
    for(i=0; i<5; i++) {
        if(args[i]!=NULL && args[i+1]==NULL) 
            args[i][strlen(args[i])-1] = '\0';
    }
    return args;
}

unsigned int get_rand_port(int *ports, int numMonitors) {
    unsigned int port=0;
    bool port_exists_flag;
    do {
        port = rand() % 15000 + 50000; // get a random port between 50.000 and 65.000
        for(int j=0; j<numMonitors; j++) { // check is this port number is used
            if(ports[j] == port) {
                port_exists_flag = true;
                break;
            }
            else 
                port_exists_flag = false;
        }
    } while(port_exists_flag);
    return port;
}

char *get_exec_args(char *exec_args,unsigned int port,long int bloomsize,long int socketBufferSize, int cyclicBufferSize, int numThreads, char * input_dir, int numMonitors, int iMonitor) {
    
    DIR *dirp;
    struct dirent *direntp;
    exec_args = malloc(2048*sizeof(char));

    

    snprintf(exec_args,2048,"-p %u -t %d -b %ld -c %d -s %ld",port, numThreads, socketBufferSize, cyclicBufferSize,bloomsize);
    unsigned int offset = strlen(exec_args);

    if((dirp=opendir(input_dir)) == NULL) {
        fprintf(stderr,"cannot open %s\n",input_dir);
        return NULL;
    }   

    char *path = calloc(100,sizeof(char)); 
    strcpy(path,input_dir); // get a string with input directory first
    int input_dir_end_index = strlen(path); // save an index to the position which input directory is ending
    int i = 0;  // index for i-th monitor
    while ((direntp = readdir(dirp)) != NULL)
    {
        if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, "..") || direntp->d_ino == 0)  // Ignore . and .. dirs
            continue;
        if(i == iMonitor) {
            char *dentry = direntp->d_name;
            unsigned int dentry_length = strlen(dentry)+2; // strlen bytes for name + 1 byte for '\0' character + 1 byte for '/' character
            snprintf(path+input_dir_end_index, dentry_length, "/%s", dentry);
            
            snprintf(exec_args+offset, 100, " %s",path); 
            offset += (strlen(path)+1); // length of path buffer + 1 byte for 'space' character
            // assign protocol 2 for sending countries to monitors
        }
        i = (i+1)%numMonitors; // round robin split of countries
    }
    closedir(dirp);
    return exec_args;
}


void free_args(char ** args) {
    for(int i=0; i<6; i++) {
        if(args[i]!=NULL)   
            free(args[i]);
    }
    free(args);

}

virusListNode *get_country_node(List *list, int *monitor_i,char *country, char *virusName) {
    Listnode *listnode = list->head;
    virusParentNode *vpnode;
    virusListNode *vnode;
    char *token, *rest, *temp_str;

    while(listnode!=NULL) {
        vpnode = (virusParentNode *)listnode->item;
        temp_str = strdup(vpnode->country);
        rest = temp_str;
        while(token = strtok_r(rest," ",&rest)) {
            if(!strcmp(token,country)) {
                vnode = vpnode->vlist->head;
                while(vnode != NULL) {
                    if(!strcmp(vnode->virusName,virusName)) {
                        free(temp_str);
                        return vnode;
                    }
                    vnode = vnode->next;
                }
            }
        }
        free(temp_str);
        (*monitor_i)--;
        listnode = listnode->next;
    }
    //free(rest);
    return NULL;
}

int get_country_node_index(List *list, int *monitor_i,char *country) {
    Listnode *listnode = list->head;
    virusParentNode *vpnode;
    char *token, *rest, *temp_str;

    while(listnode!=NULL) {
        vpnode = (virusParentNode *)listnode->item;
        temp_str = strdup(vpnode->country);
        rest = temp_str;
        while(token = strtok_r(rest," ",&rest)) {
            if(!strcmp(token,country)) {
                free(temp_str);
                return 1;
            }
        }
        free(temp_str);
        (*monitor_i)--;
        listnode = listnode->next;
    }
    //free(rest);
    return 0;
}

int clist_add_request(List *list,bool accepted,char *request_date, char *country, char *virusName) {
    Listnode *listnode = list->head;
    virusParentNode *vpnode;
    virusListNode *vnode;
    req_stat *req_stat_data;

    while(listnode!=NULL) {
        vpnode = (virusParentNode *)listnode->item;
        if(!strcmp(vpnode->country,country)) {
            vnode = vpnode->vlist->head;
            while(vnode != NULL) {
                if(!strcmp(vnode->virusName,virusName)) {
                    //find the node for this virus
                    req_stat_data = calloc(1,sizeof(req_stat));
                    req_stat_data->accepted = accepted;
                    req_stat_data->date = strdup(request_date);
                    req_stat_data->date = reverse_date(req_stat_data->date);
                    listpush(vnode->req_stats,req_stat_data);
                    return 0;
                }
                vnode = vnode->next;
            }
            virusListInsert(vpnode->vlist,virusName);
            return 1;
            //clist_add_request(list,accepted,request_date, country, virusName); // call recursively, to insert the new stats into the struct
        }
        listnode = listnode->next;
    }
    //if we didnt find the country, create a country node 
    vpnode = calloc(1,sizeof(virusParentNode));
    vpnode->country = strdup(country);
    vpnode->vlist = virusListCreate();
    listpush(list,vpnode);
    return 1;
    //clist_add_request(list,accepted,request_date, country, virusName); // call recursively, to insert the new stats into the struct
}

virusListNode *get_vnode(List *list, char *country, char *virusname) {
    Listnode *listnode = list->head;
    virusParentNode *vpnode;
    virusListNode *vnode;
    char *token, *rest;

    while(listnode!=NULL) {
        vpnode = (virusParentNode *)listnode->item;
        char *tempstr = strdup(vpnode->country);
        rest = tempstr;
        while(token = strtok_r(rest," ",&rest)) {
            //printf("%s %s\n",token,country);
            if(!strcmp(token,country)) {
                vnode = vpnode->vlist->head;
                while(vnode != NULL) {
                    if(!strcmp(vnode->virusName,virusname)) {
                        return vnode;
                    }
                    vnode = vnode->next;
                }
            }
        }
        free(tempstr);
        listnode = listnode->next;
    }
    return NULL;
}
char *get_stats_all_countries(List *list,char *reversedDate1,char *reversedDate2,char *virusname) {
    Listnode *listnode = list->head;
    virusParentNode *vpnode;
    virusListNode *vnode;
    char *token, *rest;
    char *output = NULL;
    unsigned long request = 0, accepted = 0, rejected = 0;


    while(listnode!=NULL) {
        vpnode = (virusParentNode *)listnode->item;
        vnode = vpnode->vlist->head;
        while(vnode != NULL) {
            if(!strcmp(vnode->virusName,virusname)) {
                Listnode *tempnode = vnode->req_stats->head;
                while(tempnode!=NULL) {
                    req_stat *request_statistics = (req_stat *) tempnode->item;
                    request++;
                    if(strcmp(request_statistics->date,reversedDate1)>0 &&  strcmp(request_statistics->date,reversedDate2)<0 && request_statistics->accepted)
                        accepted++;
                    else
                        rejected++;
                    tempnode = tempnode->next;
                }
            }
            vnode = vnode->next;
        }
        listnode = listnode->next;
    }
    if(request!=0) {
        output = calloc(100,sizeof(char));
        snprintf(output,100,"Total Requests: %lu\nAccepted: %lu\nRejected: %lu",request,accepted,rejected);
    }
    return output;
}

char *get_input_dir(char *path) {
    char *input_dir, *token, *rest, *tempstr;

    tempstr = strdup(path+2);
    rest = tempstr;
    token = strtok_r(rest,"/",&rest);
    input_dir = strdup(token);
    free(tempstr);
    return input_dir;
}
