#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "slist.h"

#define PROPABILITY 0.5



skipList *skipListCreate()
{
    skipList *list=NULL;
    list = (skipList *) calloc(1,sizeof(skipList));
    list->level=0;
    list->maxLevel=17; // with propability p=0.5 , skip list will include 131072 nodes
    list->size=0;

    list->head = (skipListNode *) calloc(1,sizeof(skipListNode));
    list->head->key = strdup("0000");
    //list->head->arr = (skipListNode **) calloc(list->maxLevel,sizeof(skipListNode *))
    list->head->arr = (skipListNode **) calloc(list->maxLevel+1,sizeof(skipListNode *));


    return list;
}

int randomHeight()
{
    return rand()%16+1;
}

bool increaseHeight()
{
    int k=100000;
    if( ( rand()%k) < ((double)k*PROPABILITY) )
        return true;
    return false;
}

void skipListInsert(skipList *list,void *item)
{   
    int i;
    Record *rec = (Record *) item;
    skipListNode *temp=list->head, *newnode=NULL;
    //srand(time(NULL));
    //we need an array to store all the pointers of the node that we are about to change
    skipListNode **pointersArr = (skipListNode **) calloc(list->maxLevel+1,sizeof(skipList *));

    for(i=list->level; i>=0; i--) {
        //iterate through keys
        while(temp->arr[i]!=NULL && (strcmp(temp->arr[i]->key, rec->citizenID) < 0))
            temp=temp->arr[i]; 
        pointersArr[i] = temp;
    }

    temp = temp->arr[0];
    if(temp==NULL || strcmp(temp->key, rec->citizenID)!=0) {

        list->size++;
        newnode = (skipListNode *) calloc(1,sizeof(skipListNode));
        newnode->arr = (skipListNode **) calloc(list->maxLevel+1,sizeof(skipListNode *));
        newnode->level=-1;
        newnode->key=strdup(rec->citizenID);
        newnode->item = rec;

        for(int i=0; i<=list->maxLevel; i++) {
            if(increaseHeight()) {
                (newnode->level)++;
                if(list->level < newnode->level) {
                    pointersArr[newnode->level] = list->head;
                    list->level=newnode->level;
                }
                newnode->arr[newnode->level] = pointersArr[newnode->level]->arr[newnode->level];
                pointersArr[newnode->level]->arr[newnode->level] = newnode;

            }
        }
    }   
    free(pointersArr);
}

void skipListInsertAll(skipList *list, HThash hash, char *virusname, bool flag)
{
    unsigned long i, j;
    Listnode *temp=NULL;
    Record *rec=NULL;

    for(i=0; i<hash.size; i++) {
        if( hash.arr[i] != NULL) {
            temp = hash.arr[i]->head;
            while( temp!=NULL) {
                rec = (Record *) temp->item;
                if(!strcmp(virusname, rec->virusname) && rec->vaccinated); // insert only the records with the given virus name
                    skipListInsert(list,rec);
                temp=temp->next;
            }
        }
    }
}

void skipListSearch(skipList *list, char *citizenID)
{
    
    Record *rec=NULL;
    skipListNode *temp=NULL;

    if(list==NULL)  
        return;

    temp = list->head->arr[0];

    if(temp==NULL)
        return;
      

    for(int i=list->level; i>=0; i--) {
        while(temp->arr[i]!=NULL && (strcmp(temp->arr[i]->key,citizenID)<0)) 
            temp=temp->arr[i]; 

    }
    //if only 1 record exists in skip list, then skip this step
    if(list->size!=1)
        temp=temp->arr[0];
    if(temp!=NULL && !strcmp(temp->key,citizenID)) {
        rec = (Record *) temp->item;
        if(rec->vaccinated) {
            printf("VACCINATED ON %s\n",rec->dateVaccinated);         
            return;
        }
        else {
            printf("NOT VACCINATED\n");
            return;
        }

    }
    // if this skip list refers to 'not_vaccinated_persons', then we dont want to print anything
    rec = list->head->arr[0]->item;
    if(rec->vaccinated)
        printf("NOT VACCINATED\n");
    else
        printf("\n");
}
bool skipListSearchbool(skipList *list, char *citizenID, char *date1, char *date2)
{
    
    Record *rec=NULL;
    skipListNode *temp=NULL;

    if(list==NULL)  
        return false;
        
    temp = list->head->arr[0];

    if(temp==NULL) 
        return false;

    for(int i=list->level; i>=0; i--) {
        while(temp->arr[i]!=NULL && (strcmp(temp->arr[i]->key,citizenID)<0)) 
            temp=temp->arr[i]; 

    }
    if(list->size!=1)
        temp=temp->arr[0];
    if(temp!=NULL && !strcmp(temp->key,citizenID)) {
        rec = (Record *) temp->item;
        if((strcmp(date1, rec->dateVaccinated)<0))
        if(rec->vaccinated && (strcmp(date1, rec->dateVaccinated)<0) && (strcmp(date2, rec->dateVaccinated)>0)) {
            return true;
        }
    }

    return false;
}

bool skipListSearchVACCINATENOW(skipList *list, char *citizenID)
{
    Record *rec=NULL;
    skipListNode *temp=NULL;
    if(list==NULL)  
        return false;
    temp = list->head->arr[0];
    if(temp==NULL) 
        return false;
    for(int i=list->maxLevel; i>=0; i--) {
        while(temp->arr[i]!=NULL && (strcmp(temp->arr[i]->key,citizenID)<0)) 
            temp=temp->arr[i]; 

    }
    if(list->size!=1)
        temp=temp->arr[0];
    if(temp!=NULL && !strcmp(temp->key,citizenID)) {
        rec = (Record *) temp->item;
        if(rec->vaccinated ) {
            printf("ERROR: CITIZEN %s ALREADY VACCINATED ON %s\n",rec->citizenID, rec->dateVaccinated);
            return true;
        }
    }
    return false;
}

void skipListPrint(skipList *list)
{
    Record *rec=NULL;
    skipListNode *temp=NULL;

    temp = list->head->arr[0];
    while(temp!=NULL) {
        rec = (Record *) temp->item;
        printf("%s %s %s %s %d\n",rec->citizenID, rec->firstname, rec->lastname, rec->country, rec->age);
        temp=temp->arr[0];
    }
}

void skipListDelete(skipList *list, char *key)
{   
    int i;
    Record *rec=NULL;
    skipListNode *temp=list->head->arr[0];
    //we need an array to store all the pointers of the node that we are about to change
    skipListNode **pointersArr = (skipListNode **) calloc(list->maxLevel+1,sizeof(skipList *));
    for(i=list->level; i>=0; i--) {
        //iterate through keys
        while(temp->arr[i]!=NULL && (strcmp(temp->arr[i]->key, key) < 0))
            temp=temp->arr[i]; 
        pointersArr[i] = temp;
    }
    if(list->size!=1)
        temp = temp->arr[0];
    if(temp!=NULL && !strcmp(temp->key,key)) {
        for(int i=0; i<=list->maxLevel; i++) {
            if(pointersArr[i]->arr[i]!=temp) 
                break;
            pointersArr[i]->arr[i] = temp->arr[i];
        }
        while(list->head->arr[list->level] == NULL && list->level>0)
            list->level--;
    }   
}

void skipListDestroy(skipList *list)
{
    if(list!=NULL) {
        skipListNode *temp=NULL;
        while(list->head!=NULL) {
            temp=list->head->arr[0];
            free(list->head->arr);
            free(list->head->key);
            free(list->head);
            list->head=temp;
        }
        free(list);
    }
}