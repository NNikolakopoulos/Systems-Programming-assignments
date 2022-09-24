#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "hash.h"
#include "list.h"
#include "slist.h"
#include "msg.h"
#include "bloom.h"
#include "logs.h"
#include "global.h"
#include "functions.h"

static bool code_4_setup;


int code_1(char *msg, HThash hash, virusList *list, int writefd);
int code_2(char *msg, unsigned long msg_length,HThash hash, virusList *list);
int code_3(char *msg, unsigned long msg_length, List *list,int monitor);
int code_4(char *msg, unsigned long msg_length,HThash *hash, virusList *list, int writefd);
int code_5(char *msg, unsigned long msg_length,HThash hash, virusList *list, int writefd);
int code_6(char *msg, unsigned long msg_length, List *list, char **args);
int code_a(char *msg,unsigned long msg_length);
int code_b(char *msg,unsigned long msg_length,HThash hash, virusList *list, int writefd);
int code_c(char *msg);
int code_d(char *msg, unsigned long msg_length, int *pipesfds, int monitor_i, List *list);
int code_parent_0(char *msg, unsigned long msg_length);
char *copy_word_from_buf(char *msg);
unsigned long get_bloom_info_from_buf(char *msg);



int protocol_commands_monitor(char *package, HThash hash, virusList *list, int writefd) {
    char *msg;
    char protocol;
    unsigned long msg_length;

    protocol = get_protocol(package);
    msg_length = get_package_length(package);
    msg = get_msg(package);
    switch(protocol) {
        case '1':
            code_1(msg,hash,list,writefd);
        break;
        case '2':
            code_2(msg,msg_length,hash,list);
        break;
        case '4':
            code_4(msg,msg_length,&hash,list,writefd);
        break;
        case '5':
            code_5(msg,msg_length,hash,list,writefd);
        break;
        case '9':
            free(package);
            return 9;
        break;
        case 'a':
            code_a(msg,msg_length);
        break;
        case 'b':
            code_b(msg,msg_length,hash,list,writefd);
        break;

    }
    free(package);
    return 0;
}

int protocol_commands_parent(char *package, int *fds, List *list,int monitor, char **args, List *clist) {
    char *msg;
    char protocol;
    unsigned long msg_length;

    protocol = get_protocol(package);
    msg_length = get_package_length(package);
    msg = get_msg(package);
    switch(protocol) {
        case '0':
            //  signal that monitor are ready
            if (code_parent_0(msg,msg_length) == -1)
                return -1;
        break;
        case '3':
            code_3(msg,msg_length,list,monitor);
        break;
        case '6':
            if (code_6(msg,msg_length,clist,args) == 1) {
                free(package);
                return 1;
            }
            else {
                free(package);    
                return 0;
            }
        break;
        case '7': {
            free(package);
            return 7;
        }
        break;
        case 'c':
            code_c(msg);
        break;
        case 'd':
            code_d(msg,msg_length,fds,monitor,list);
        break;
        case 'z': {
            free(package);
            return 10;
        }
        break;

    }
    free(package);
    return 0;
}

int code_parent_0(char *msg, unsigned long msg_length) {
    if(!strcmp(msg,"monitor init ready")) 
        return 0;
    return -1;
}


// this code refers to bloom size , pipe buffer size and input directory
int code_1(char *msg, HThash hash, virusList *list, int writefd) {
    char *path, *newpath,*token, *ptr = msg;

    token = strtok_r(ptr," ",&ptr); // token = bloom size
    list->bloomsize=atol(token); // create the head for virus list (later we will insert skip lists, bloom filters for each virus)

    token = strtok_r(ptr," ",&ptr); // token = size of buffer for pipes
    list->pipe_buf_size = atol(token);


    token = strtok_r(ptr," ",&ptr); // token = the input directory
    list->input_dir = malloc((strlen(token)+1)*sizeof(char));
    strcpy(list->input_dir,token);

    //send a message to parent , to inform him that he is ready to get countries 
    char ready_msg[]="monitor init ready";
    if(pipe_write(writefd,ready_msg,strlen(ready_msg),'0') == -1) 
            return -1;

    code_4_setup = true;

    return 0;
}

int code_2(char *msg, unsigned long msg_length,HThash hash, virusList *list) {
    int newpath_length = (strlen(list->input_dir)+msg_length+2); // (assume that the max length of country name is 20 + 1 char for '/' + 1 char for '\0')
    char *newpath = calloc( newpath_length ,sizeof(char)); // newpath = " ./input_dir/country " 
    //printf("%s\n",hash.input_dir);
    snprintf(newpath,newpath_length,"%s/%s",list->input_dir,msg);

    hash = HTinsertfiles(hash, list,newpath);

    add_country_global_struct(msg);
}

char *copy_word_from_buf(char *msg) {
    char *word = malloc(20*sizeof(char));
    int i = 0;
    while(msg[i] != ';') {
        word[i] = msg[i];
        i++;
    }
    word[i] = '\0';
    
    return word;
}

unsigned long get_bloom_info_from_buf(char *msg) {
    char *info = malloc(10*sizeof(char));
    unsigned long n;

    int i = 0;
    while(i<9) {
        info[i] = msg[i];
        i++;
    }
    info[9] = '\0';
    n = atol(info);
    free(info);
    return n;
}

int code_3(char *msg, unsigned long msg_length, List *list,int monitor) {
    char *virus;
    unsigned long bloom_offset,nbytes;
    int i, msg_offset = 0;
    monitor = list->size - monitor; // this is a stack(inserting nodes at start), so we are counting backwards
    // get the virus name
    virus = copy_word_from_buf(msg);

    msg_offset = strlen(virus)+1;
    // get the bloom filter offset
    bloom_offset = get_bloom_info_from_buf(msg+msg_offset);

    msg_offset += 9; 
    //get the number of bytes to copy
    nbytes = get_bloom_info_from_buf(msg+msg_offset);

    msg_offset += 9;
    Listnode *node=list->head;
    //printf("%s %s %ld %ld\n",msg,virus,bloom_offset,msg_offset);
    // go to this monitor's node
    for(i=1; i<monitor; i++) {
        node = node->next;
    }
    

    virusParentNode *vpnode = (virusParentNode *) node->item;
    virusListNode *vnode = vpnode->vlist->head;

    while(vnode!=NULL ) {
        if(!strcmp(vnode->virusName,virus))
            break;
        vnode = vnode->next;
    }
    //if there is not a virus list node for this virus, create it.
    if(vnode == NULL) {
        virusListInsert(vpnode->vlist, virus);
        vpnode = (virusParentNode *) node->item;
        vnode = vpnode->vlist->head;
    }

    //printf("%ld %ld\n",nbytes, bloom_offset);
    memcpy((vnode->bloom->arr)+bloom_offset,msg+msg_offset,nbytes);
    //printf("%d\n",vnode->bloom->arr[bloom_offset]);
    free(virus); 
}

int code_4(char *msg, unsigned long msg_length,HThash *hash, virusList *list, int writefd) {
    if(hash->not_changed) //if no changed occured , then 
        return 0;
    if(code_4_setup) { // if it is called during set up(the first time)
        //after finishing the initialization part,
        // first create log file for this monitor
        create_logs(*hash,list->pipe_buf_size);

        code_4_setup = false;
    }

    // then  we need to send to parent the bloom filters
    char *pipe_buf = malloc((list->pipe_buf_size)*sizeof(char));
    unsigned long offset = 0, nbytes = 0, actual_pipe_size, pipe_offset = 0;
    virusListNode *node = list->head;
    bool break_flag=false;

    // for every virus , send each bloom filter to parent
    while(node != NULL) {
        
        

        //printf("%s\n",node->virusName);
        //snprintf(pipe_buf+10,strlen(node->virusName)+2,"%s;",node->virusName); // the message will have the name of the virus first,
        pipe_offset = (29 + strlen(node->virusName)); // n=? bytes for virus_name, 1 byte for ';' ,9 bytes for bloom filter offset, 9 bytes for number of bytes of bloom filter sent
        actual_pipe_size = list->pipe_buf_size - 1 - pipe_offset; // 
        offset=0;
        break_flag = false;

        while(offset < list->bloomsize) {
            memset(pipe_buf,0,(list->pipe_buf_size));
            //we are sending "pipe_buf_size" bytes at a time
            if( (offset + actual_pipe_size) >= list->bloomsize) {
                //nbytes = ((actual_pipe_size + offset) - list->bloomsize); // if we exceed the array, send only the remaining cells
                nbytes = (list->bloomsize - offset ); // if we exceed the array, send only the remaining cells
                break_flag = true;
            }
            else
                nbytes = actual_pipe_size;   // else send as much bytes, as pipe buffer can transfer

            unsigned long final_pipe_buf_length = 30+nbytes+strlen(node->virusName);
            //printf("%s %ld %ld\n",pipe_buf,offset,list->bloomsize);
            pipe_buf[0] = '3';
            snprintf(pipe_buf+1,10,"%09ld",final_pipe_buf_length-10); 
            snprintf(pipe_buf+10,strlen(node->virusName)+2,"%s;",node->virusName);
            snprintf(pipe_buf+strlen(node->virusName)+11,10,"%09ld",offset); // bloom filter offset
            snprintf(pipe_buf+strlen(node->virusName)+20,10,"%09ld",nbytes); //  number of bloom filter bytes
            memcpy(pipe_buf+pipe_offset,(node->bloom->arr)+offset,nbytes); // a part of bloom filter
            //pipe_buf[final_pipe_buf_length] = '\0';


            if(safe_write(writefd,pipe_buf,final_pipe_buf_length) == -1) { 
                //if( errno == EAGAIN  || errno == EWOULDBLOCK) //try again;
                    //continue;
                printf("sending bloom filters failed\n");
                free(pipe_buf);
                return -1;
            }
            if(break_flag)
                break;
            //printf("%s\n",offset);
            offset+=nbytes;
        }
        //printf("%s  ENDDDD~~~~~~~\n",node->virusName);
        
        node = node->next;
    }
    free(pipe_buf);
    if(pipe_write(writefd,"",1,'7') == -1)  // assign protocol 7 to tell parent that he sent all bloom filters
        return -1;   
    hash->not_changed = true; 
    
}

char *reverse_date(char *temp) {
    char *date = malloc(11*sizeof(char));

    date[0]=temp[6]; date[1]=temp[7]; date[2]=temp[8]; date[3]=temp[9];
    date[4]=temp[5]; 
    date[5]=temp[3]; date[6]=temp[4]; 
    date[7]=temp[2];
    date[8]=temp[0]; date[9]=temp[1]; 
    date[10]='\0';

    free(temp);
    return date;
}

char *last_months(char *date) {
    char *newdate = calloc(11,sizeof(char));
    char *ddstr, *mmstr, *yyyystr;    
    int dd, mm, yyyy;

    ddstr = calloc(3,sizeof(char));
    mmstr = calloc(3,sizeof(char));
    yyyystr = calloc(5,sizeof(char));
    
    
    strncpy(yyyystr,date,4);
    strncpy(mmstr,date+5,2);
    strncpy(ddstr,date+8,2);


    dd = atoi(ddstr);
    mm = atoi(mmstr);
    yyyy = atoi(yyyystr);

    if(mm-6 <= 0) {
        int remainder = 6-mm;
        yyyy -= 1;
        mm = 12-remainder;
    }
    else
        mm -= 6;
    
    snprintf(newdate,11,"%02d-%02d-%04d",dd,mm,yyyy);

    return newdate;
}

int code_5(char *msg, unsigned long msg_length,HThash hash, virusList *list, int writefd) {
    char *token, *rest, *citizenid, *date, *virusname, *country, *lastmonths, *pipe_buf;

    rest = msg;

    token = strtok_r(rest," ",&rest);
    citizenid = malloc(5*sizeof(char));
    strcpy(citizenid,token);

    token = strtok_r(rest," ",&rest);
    date = malloc(11*sizeof(char));
    strcpy(date,token);
    date = reverse_date(date);
    //lastmonths = reverse_date(lastmonths);

    token = strtok_r(rest," ",&rest);
    country = malloc((strlen(token)+1)*sizeof(char));
    strcpy(country,token);

    token = strtok_r(rest," ",&rest);
    virusname = malloc((strlen(token)+1)*sizeof(char));
    strcpy(virusname,token);

    Listnode *listnode=hash.countryList->head, *templistnode;
    countryNode *cnode;
    countryVirusNode *cvnode;

    while(listnode != NULL) {
        cnode = (countryNode *) listnode->item; // find the country node first
        if(strcmp(cnode->country,country)==0) {
            templistnode = cnode->viruses->head;
            while(templistnode != NULL) {
                cvnode = (countryVirusNode *) templistnode->item;  // then find the node with this virus for this particular country
                if(!strcmp(cvnode->virusname,virusname)) {
                    char *travel_req_retval;
                    travel_req_retval = skipListSearchTravelReq(cvnode->citizens,citizenid,date);
                    if(travel_req_retval!=NULL) {
                        pipe_buf = malloc(26*sizeof(char));
                        snprintf(pipe_buf,26,"YES %s %s",travel_req_retval, date);
                        if(pipe_write(writefd,pipe_buf,26,'6') == -1) //assign protocol 6 to send parent back the travel request response
                            return -1;
                        free(pipe_buf);
                    }
                    else {
                        pipe_buf = malloc(3*sizeof(char));
                        strcpy(pipe_buf,"NO");
                        if(pipe_write(writefd,pipe_buf,3,'6') == -1) //assign protocol 6 to send parent back the travel request response
                            return -1;
                        free(pipe_buf);
                    }
                }
                templistnode = templistnode->next;
            }
        }
        
        listnode = listnode->next;
    }
}

int code_6(char *msg, unsigned long msg_length, List *list,char **args) {
    char *rest, *token, *dateVaccinated, *lastMonthsDateVaccinated;
    bool accepted;
    
    rest = msg;
    token = strtok_r(rest," ",&rest);
    if(!strcmp(token,"NO")) {
        printf("REQUEST REJECTED – YOU ARE NOT VACCINATED\n");
        accepted = false;
    }
    else {
         token = strtok_r(rest," ",&rest);
        dateVaccinated = calloc(11,sizeof(char));
        strncpy(dateVaccinated,token,10);

        token = strtok_r(rest," ",&rest);
        lastMonthsDateVaccinated = calloc(11,sizeof(char));
        strncpy(lastMonthsDateVaccinated,token,10);

        
        if( strcmp(dateVaccinated,lastMonthsDateVaccinated) > 0) {
            printf("REQUEST REJECTED – YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE\n");
            accepted = false;
        }
        else {
            printf("REQUEST ACCEPTED – HAPPY TRAVELS\n");
            accepted = true;
        }
        
    }

    free(dateVaccinated);
    free(lastMonthsDateVaccinated);
   
    add_country_global_struct(args[4]);
    add_request_global_struct(accepted,args[4]);
    if(clist_add_request(list, accepted, args[2], args[4], args[5]) == 1) 
        if(clist_add_request(list, accepted, args[2], args[4], args[5]) == 1) 
            clist_add_request(list, accepted, args[2], args[4], args[5]);

    return accepted;
}

int code_a(char *msg,unsigned long msg_length) {
    char *token, *rest, *date, *country, *acceptedStr, *pipe_buf;
    int accepted;

    rest = msg;

    token = strtok_r(rest," ",&rest);
    date = malloc(11*sizeof(char));
    strcpy(date,token);
    date = reverse_date(date);

    token = strtok_r(rest," ",&rest);
    country = malloc((strlen(token)+1)*sizeof(char));
    strcpy(country,token);


    token = strtok_r(rest," ",&rest);
    acceptedStr = malloc((strlen(token)+1)*sizeof(char));
    strcpy(acceptedStr,token);
    accepted = atoi(acceptedStr);

    add_request_global_struct(accepted, country);

    free(date);
    free(country);
    free(acceptedStr);

}


int code_b(char *msg,unsigned long msg_length,HThash hash, virusList *list, int writefd) {
    List *HTentry;
    Listnode *listnode;
    Record *rec;
    char *pipe_buf = calloc(list->pipe_buf_size,sizeof(char));
    bool first_time = true;

    HTentry = HTget_entry(hash,msg);
    listnode = HTentry->head;

    while(listnode!=NULL) {
        rec = (Record *) listnode->item;
        if(!strcmp(rec->citizenID,msg)) {
            if(first_time){
                snprintf(pipe_buf,list->pipe_buf_size,"%s %s %s %s\nAGE %d",rec->citizenID,rec->firstname, rec->lastname, rec->country,rec->age);
                if(pipe_write(writefd,pipe_buf,strlen(pipe_buf),'c') == -1) //  assign protocol 'c' to send parent basic info about the citizen 
                    return -1;
                first_time = false;
            }
            if(rec->vaccinated)
                snprintf(pipe_buf,list->pipe_buf_size,"%s VACCINATED ON %s",rec->virusname,rec->dateVaccinated);
            else
                snprintf(pipe_buf,list->pipe_buf_size,"%s NOT YET VACCINATED",rec->virusname);
            if(pipe_write(writefd,pipe_buf,strlen(pipe_buf),'c') == -1) // protocol 'd' to send parent vaccianation records
                    return -1;

        }
        listnode = listnode->next;
    }
    //if we found this citizen, in this monitor, send "end" message to parent
    if(!first_time) {
        if(pipe_write(writefd,"",0,'7') == -1) // protocol '7' to end the communication between parent and child
            return -1;
    }
    else
        if(pipe_write(writefd,"",0,'z') == -1) // protocol 'z' to tell parent that this monitor didnt found any info about this citizen
            return -1;
}


int code_c(char *msg) {
    printf("%s\n",msg);
}

int code_d(char *msg, unsigned long msg_length, int *pipesfds, int monitor_i, List *list) {
    char *token, *rest;
    rest = msg;
    while(token = strtok_r(rest," ",&rest)) {
            if(pipe_write(pipesfds[2*monitor_i+1],token,strlen(token),'2') == -1) {
            perror("error while sending country to monitor for handling usr1");
            exit(1);
        }
    }
    
    if(pipe_write(pipesfds[2*monitor_i+1],"",0,'4') == -1) { // tell monitors that parent is ready to get bloom filters
        perror("error while sending ready for bloom to monitor for handling usr1");
        exit(1);
    }
    while(1) {
        char *package;
        package = pipe_read(pipesfds[2*monitor_i]);
        if(package == NULL)
            continue;
        if(protocol_commands_parent(package,NULL,list,monitor_i,NULL,NULL) == 7) 
            break;
    }
}