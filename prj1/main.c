#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "time.h"

#include "hash.h"
#include "bloom.h"
#include "list.h"
#include "slist.h"
#include "functions.h"

int main(void) {
    HThash hash;
    virusList *list=NULL;

    size_t len=100;
    int age=0, begin, lenn, end;
    char *buf = NULL, *token = NULL, *citizenid = NULL, *fname = NULL, *lname = NULL, *virusname = NULL, *country = NULL, *date1 = NULL, *date2 = NULL, *vaccinatedstr = NULL, *dateVaccinated = NULL, *temp = NULL;
    buf = (char *) calloc(len,sizeof(char));
    citizenid = (char *) calloc(15,sizeof(char));
    fname = (char *) calloc(13,sizeof(char));
    lname = (char *) calloc(13,sizeof(char));
    virusname = (char *) calloc(15,sizeof(char));
    date1 = (char *) calloc(10,sizeof(char));
    date2 = (char *) calloc(10,sizeof(char));
    vaccinatedstr = (char *) calloc(4,sizeof(char));
    temp = (char *) calloc(10,sizeof(char));

    //srand(time(NULL));
    
    hash=HTcreate();
    list=virusListCreate();
    hash=HTinsertAll(hash,list,"inputFile.txt");



    while(1) {
        printf("Please insert command:\n");
        getline(&buf,&len,stdin);
        token = strtok(buf," ");
        if(!strcmp(token,"/vaccineStatusBloom")) {
            token=strtok(NULL," ");
            strcpy(citizenid,token);
            //citizenid=strdup(token);
            token=strtok(NULL," ");
            token[(strlen(token)-1)]='\0';
            printf(" %s\n",token);
            vaccineStatusBloom(list, citizenid, token);
            //free(citizenID);
        }
        else if(!strcmp(token,"/vaccineStatus")) {
            token=strtok(NULL," ");
            strcpy(citizenid,token);
            //citizenid=strdup(token);
            token=strtok(NULL," ");
            if(token == NULL) {
                citizenid[(strlen(citizenid)-1)]='\0';
                vaccineStatusAll(list, citizenid);
            }
            else {
                token[(strlen(token)-1)]='\0';
                vaccineStatus(list, citizenid, token);
            }
        }
        else if(!strcmp(token,"/populationStatus")) {
            token=strtok(NULL," ");
            //by default the country is NULL, so if the country is not given, then we will check for all of them
            country = NULL;
            //if country is given, then copy it
            if(token[0]=='[') {
                country = (char *) calloc(15,sizeof(char));
                token++;
                strncpy(country, token, strlen(token));
                country[strlen(token)-1]='\0';
                token=strtok(NULL," ");
            }
            if(token!=NULL) {
                strcpy(virusname,token);
                //virusname = strdup(token);
                token=strtok(NULL," ");
                if(token != NULL) {
                    strcpy(temp,token);
                    // reverse and save date1
                    lenn=strlen(temp)-1;
                    for ( begin = 0; begin < lenn; begin++) {
                        date1[begin] = temp[end];
                        end--;
                    }
                    date1[0]=temp[6]; date1[1]=temp[7]; date1[2]=temp[8]; date1[3]=temp[9];
                    date1[4]=temp[5]; 
                    date1[5]=temp[3]; date1[6]=temp[4]; 
                    date1[7]=temp[2];
                    date1[8]=temp[0]; date1[9]=temp[1]; 
                    date1[10]='\0';
                    token=strtok(NULL," ");
                    if(token==NULL) 
                        printf("Error: Only 1 date given\n");
                    else {
                        
                        // do the same for date 2
                        strcpy(temp,token);
                        date2[0]=temp[6]; date2[1]=temp[7]; date2[2]=temp[8]; date2[3]=temp[9];
                        date2[4]=temp[5]; 
                        date2[5]=temp[3]; date2[6]=temp[4]; 
                        date2[7]=temp[2];
                        date2[8]=temp[0]; date2[9]=temp[1]; 
                        date2[10]='\0';
                        //date2=strdup(token);
                        populationStatus(hash,list, virusname, date1, date2, country);
                    }
                }
                else {
                    virusname[(strlen(virusname)-1)]='\0';
                    populationStatus(hash,list, virusname, NULL, NULL, country);
                }   
                    
            }
            else
                printf("Please give valid input!\n");
            free(country);
        }
        else if(!strcmp(token,"/popStatusByAge")) {
            token=strtok(NULL," ");
            //by default the country is NULL, so if the country is not given, then we will check for all of them
            country = NULL;
            //if country is given, then copy it
            if(token[0]=='[') {
                country = (char *) calloc(15,sizeof(char));
                token++;
                strncpy(country, token, strlen(token));
                country[strlen(token)-1]='\0';
                token=strtok(NULL," ");
            }
            if(token!=NULL) {
                strcpy(virusname,token);
                token=strtok(NULL," ");
                if(token != NULL) {
                    strcpy(temp,token);
                    temp[strlen(temp)-1]='\0';
                    date1[0]=temp[6]; date1[1]=temp[7]; date1[2]=temp[8]; date1[3]=temp[9];
                    date1[4]=temp[5]; 
                    date1[5]=temp[3]; date1[6]=temp[4]; 
                    date1[7]=temp[2];
                    date1[8]=temp[0]; date1[9]=temp[1]; 
                    date1[10]='\0';

                    token=strtok(NULL," ");
                    if(token==NULL) 
                        printf("Error: Only 1 date given\n");
                    else {
                        strcpy(temp,token);
                        temp[strlen(temp)-1]='\0';
                        date2[0]=temp[6]; date2[1]=temp[7]; date2[2]=temp[8]; date2[3]=temp[9];
                        date2[4]=temp[5]; 
                        date2[5]=temp[3]; date2[6]=temp[4]; 
                        date2[7]=temp[2];
                        date2[8]=temp[0]; date2[9]=temp[1]; 
                        date2[10]='\0';
                        //date2=strdup(token);
                        popStatusByAge(hash,list, virusname, date1, date2, country);
                    }
                }
                else {
                    virusname[(strlen(virusname)-1)]='\0';
                    popStatusByAge(hash,list, virusname, NULL, NULL, country);
                }   
                    
            }
            else
                printf("Please give valid input!\n");
            free(country);
        }
        else if(!strcmp(token,"/insertCitizenRecord")) {
            country = (char *) calloc(15,sizeof(char));
            token=strtok(NULL," ");
            strcpy(citizenid,token);
            token=strtok(NULL," ");
            if(token != NULL) {
                strcpy(fname,token);
                token=strtok(NULL," ");
                if(token != NULL) {
                    strcpy(lname,token);
                    token=strtok(NULL," ");
                    if(token != NULL) {
                        strcpy(country,token);
                        token=strtok(NULL," ");
                        if(token != NULL) {
                            age=atoi(token);
                            token=strtok(NULL," ");
                            if(token != NULL) {
                                strcpy(virusname,token);
                                token=strtok(NULL," ");
                                if(token != NULL) {
                                    strcpy(vaccinatedstr,token);
                                    dateVaccinated = NULL;
                                    if(!strcmp(vaccinatedstr,"YES")) {
                                        token=strtok(NULL," ");
                                        if(token != NULL) {
                                            dateVaccinated = strndup(token, (strlen(token)-1));
                                        }
                                        else {
                                            printf("Please give date!");
                                            free(country);
                                            continue;
                                        }
                                    }
                                    insertCitizenRecord(hash, list, citizenid, fname, lname, country, age, virusname, vaccinatedstr, dateVaccinated);
                                   // printf("Citizen: %s  inserted succesfully\n",citizenid);
                                    free(country);
                                    free(dateVaccinated);
                                    continue;
                                }
                            }
                        }
                    }
                }
            }
            printf("Invalid input!");
            free(country);
        }
        else if(!strcmp(token,"/vaccinateNow")) {
            country = (char *) calloc(15,sizeof(char));
            token=strtok(NULL," ");
            strcpy(citizenid,token);
            token=strtok(NULL," ");
            if(token != NULL) {
                strcpy(fname,token);
                token=strtok(NULL," ");
                if(token != NULL) {
                    strcpy(lname,token);
                    token=strtok(NULL," ");
                    if(token != NULL) {
                        strcpy(country,token);
                        token=strtok(NULL," ");
                        if(token != NULL) {
                            age=atoi(token);
                            token=strtok(NULL," ");
                            if(token != NULL) {
                                strncpy(virusname,token,strlen(token));
                                virusname[strlen(token)-1]='\0';
                                vaccinateNow(hash, list, citizenid, fname, lname, country, age, virusname);
                                //printf("Citizen: %s  inserted succesfully\n",citizenid);
                                free(country);
                                continue;
                                
                            }
                        }
                    }
                }
            }
            printf("Invalid input!");
            free(country);
        }
        else if(!strcmp(token,"/list-nonVaccinated-Persons")) {
            token=strtok(NULL," ");
            strncpy(virusname,token, strlen(token));
            virusname[strlen(token)-1]='\0';
            listnonVaccinatedPersons(list, virusname);
        }
        else if(!strcmp(token,"/exit\n")) {
            printf("Exiting...\n");
            HTdestroy(hash);
            virusListDestroy(list);
            free(citizenid);
            free(fname);
            free(lname);
            free(vaccinatedstr);
            free(virusname);
            free(date1);
            free(date2);
            //free(token);
            //free(temp);
            break;
        }
    }

    return 0; 
}