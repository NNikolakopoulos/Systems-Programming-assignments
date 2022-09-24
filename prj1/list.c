#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>


#include "list.h"
#include "bloom.h"
#include "slist.h"

List *listcreate() {
    List *list=NULL;
    list = (List *) calloc(1,sizeof(List));
    list->head=NULL;
    list->size = 0;
    return list;
}

void listpush(List *l,void *data) 
{
    Listnode * newnode = (Listnode *) calloc(1,sizeof(Listnode));
    newnode -> item = data;
    newnode -> next = l->head;
    l->head = newnode; 
    (l->size)++;
}

void recordDelete(void *record) 
{
    Record *rec = (Record *) record;
    free(rec->citizenID);
    free(rec->firstname);
    free(rec->lastname);
    free(rec->virusname);
    free(rec->dateVaccinated);
    free(rec);
}

void listdelete(List *l) {
    Listnode * nextnode=NULL;
    while(l->head != NULL) {
        nextnode = (l->head) -> next;
        recordDelete(l->head->item);
        free(l->head);
        (l->head) = nextnode; 
    }
    free(l);
}

void listpop(List *l) {
    Listnode *nextnode=NULL;
    if(l->head != NULL) { 
        nextnode = l -> head -> next;
        recordDelete(l->head -> item);
        free(l->head);
        l->head = nextnode;
    }
}

void listprint(List *l) {
    Listnode *nextnode;
    Record *rec;
    nextnode = l->head;
    while(nextnode!=NULL) {
        rec = (Record *) nextnode->item;
        printf("%d\n", (int)rec->age);
        nextnode=nextnode->next;
    }
    printf("size: %d\n", l->size );
}

/////////////////some functions for countries list ///////////////////////


void ListCountryInsert(List *list, Record *citizen, char *countryname) 
{
    Listnode *temp=NULL;
    countryNode *c=NULL;

    temp=list->head;
    while(temp!=NULL) {
        c = (countryNode *) temp->item;
        // if this country is already inserted in the list, simply add a pointer to the referred person/citizen
        if(!strcmp(c->country,countryname)) {
            listpush(c->citizens, citizen);
            citizen->country = c->country;
            return;
        }
        temp=temp->next;
    }
    
    // we must create and insert this country node into list
    c = (countryNode *) calloc(1,sizeof(countryNode));
    c->country = strdup(countryname);
    citizen->country = c->country;
    c->citizens=listcreate();
    listpush(c->citizens,citizen);
    listpush(list,c);
}

void ListCountryIndex(List *list,Record *citizen,char *countryname) {
    Listnode *temp=NULL;
    countryNode *c=NULL;
    temp=list->head;
    while(temp!=NULL) {
        c = (countryNode *) temp->item;
        // if this country is already inserted in the list, simply add a pointer to the referred person/citizen
        if(!strcmp(c->country,countryname)) {
            citizen->country = c->country;
            listpush(c->citizens, citizen);
            return;
        }
        temp=temp->next;
    }
}

void ListCountryDelete(List *list) 
{
    Listnode *countryptr=NULL, *citizenptr=NULL;
    countryNode *countrynode=NULL;

    while(list->head!=NULL) {
        // save next country node
        countryptr=list->head->next;

        countrynode = (countryNode *) list->head->item;
        // we must also delete the citizen list for every country
        while(countrynode->citizens->head!=NULL) {
            //save the next citizen
            citizenptr=countrynode->citizens->head->next;
            free(countrynode->citizens->head);
            countrynode->citizens->head=citizenptr;
        }
        free(countrynode->country);
        free(countrynode->citizens);
        free(countrynode);
        free(list->head);
        list->head=countryptr;
    }
    free(list);
}






//////////////////////////     some functions for virus list     //////////////////
////////////////////    every virus has each own bloom filter and 2 skip lists    /////////////

virusList *virusListCreate()
{
    virusList *list=NULL;
    list = (virusList *) malloc(sizeof(virusList));
    list->head = NULL;
    list->size=0;
    return list;
}

void virusListInsert(virusList *list, char *virusname, HThash hash)
{
    virusListNode *new=NULL;
    new = (virusListNode *) malloc(sizeof(virusListNode));
    new->virusName = strdup(virusname);
    new->vaccinated_persons = NULL;
    new->not_vaccinated_persons = NULL;
    new->bloom = NULL;
    new->next = list->head;
    list->head = new;
    list->size++;

    // now create the bloom filter and skip lists
    new->bloom = bloomCreate(virusname);
    
    new->vaccinated_persons = skipListCreate();
    new->not_vaccinated_persons = skipListCreate();
}

//return true if this virus name already exists in list
virusListNode *virusListSearch(virusList *list, char *virusname)
{
    virusListNode *temp=list->head;
    while(temp != NULL) {
        if(!strcmp(virusname, temp->virusName))
            return temp;
        temp=temp->next;
    }
    return NULL;
}


void virusListDestroy(virusList *list)
{
    virusListNode *next;
    while(list->head!=NULL) {
        next = list->head->next;
        bloomDestroy(list->head->bloom);
        skipListDestroy(list->head->vaccinated_persons);
        skipListDestroy(list->head->not_vaccinated_persons);
        free(list->head->virusName);
        free(list->head);
        list->head = next;
    }
    free(list);
}

void virusListPrint(virusList *list)
{
    virusListNode *next=NULL, *temp=NULL;

    temp=list->head;
    while(temp!=NULL) {
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        printf("%s\n\n  ====VACCINATED====",temp->virusName);
        skipListPrint(temp->vaccinated_persons);
        printf("\n ===NOT =====");
        skipListPrint(temp->not_vaccinated_persons);
        temp = temp->next;
    }
}

