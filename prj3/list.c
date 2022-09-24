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

void listdeletefilesparsed(List *l) {
    Listnode * nextnode=NULL;
    while(l->head != NULL) {
        nextnode = (l->head) -> next;
        free(l->head->item);
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

void listprintstr(List *l) {
    Listnode *nextnode;
    char *rec;
    nextnode = l->head;
    while(nextnode!=NULL) {
        rec = (char *) nextnode->item;
        printf("%s\n", rec);
        nextnode=nextnode->next;
    }
    printf("size: %d\n", l->size );
}

int comparestr(void *str1, void *str2) {
    const char *s1 = (const char *) str1;
    const char *s2 = (const char *) str2;
    //printf("%s  %s ||||| ",s1,s2);
    return strcmp(s1,s2);
}

bool listsearch(List *l, void *item, int compare(void *, void *))
{
    Listnode *nextnode;
    Record *rec;
    nextnode = l->head;
    while(nextnode!=NULL) {
        if( !compare(item,nextnode->item) )
            return true;
        nextnode = nextnode->next;
    }
    return false;
}

void listprintf(List *list) {
    Listnode *nextnode;
    char *file;
    nextnode = list->head;

    while(nextnode!=NULL) {
        file = (char *) nextnode->item;
        printf("%s  ",file);
        nextnode = nextnode->next;
    }
}

/////////////////some functions for countries list ///////////////////////


void ListCountryInsert(List *list, Record *citizen, char *countryname) 
{
    Listnode *temp = NULL, *listnodetemp=NULL;
    countryVirusNode *viruslistnode = NULL;
    countryNode *c=NULL;

    temp=list->head;
    while(temp!=NULL) {
        c = (countryNode *) temp->item;
        // if this country is already inserted in the list, 
        if(!strcmp(c->country,countryname)) {
            // search for each virus in this country
            listnodetemp = (Listnode *) c->viruses->head;
            while(listnodetemp!=NULL) {
                viruslistnode = (countryVirusNode *) listnodetemp->item;
                //if there is already a virus skip list in this country
                if(!strcmp(citizen->virusname,viruslistnode->virusname)) {
                    skipListInsert(viruslistnode->citizens,citizen);
                    citizen->country = c->country;
                    return;
                }
                listnodetemp = listnodetemp->next;
            }
            // if this virus does not exist in this country, 
            // create a node for virus and its skip list
            countryVirusNode *newnode = calloc(1,sizeof(countryVirusNode));
            newnode->virusname = strdup(citizen->virusname);
            newnode->citizens = skipListCreate();
            skipListInsert(newnode->citizens, citizen);
            listpush(c->viruses,newnode);
            citizen->country = c->country;
            return;
        }
        temp=temp->next;
    }
    
    // we must create and insert this country node into list
    c = (countryNode *) calloc(1,sizeof(countryNode));
    c->country = strdup(countryname);
    citizen->country = c->country;
    c->viruses=listcreate();
    // dont forget to create a node for virus and its skip list
    countryVirusNode *newnode = calloc(1,sizeof(countryVirusNode));
    newnode->virusname = strdup(citizen->virusname);
    newnode->citizens = skipListCreate();
    skipListInsert(newnode->citizens, citizen);
    // insert citizen to skip list of this virus
    listpush(c->viruses,newnode);
    //insert this country node to the original list
    listpush(list,c);
}

void ListCountryIndex(List *list,Record *citizen,char *countryname) {
    Listnode *temp=NULL, *listnodetemp=NULL;
    countryVirusNode *viruslistnode = NULL;
    
    countryNode *c=NULL;
    temp=list->head;
    while(temp!=NULL) {
        c = (countryNode *) temp->item;
        // if this country is already inserted in the list, simply add a pointer to the referred person/citizen
        if(!strcmp(c->country,countryname)) {
            // search for each virus in this country
            listnodetemp = (Listnode *) c->viruses->head;
            while(listnodetemp!=NULL) {
                viruslistnode = (countryVirusNode *) listnodetemp->item;
                //if there is already a virus skip list in this country
                if(!strcmp(citizen->virusname,viruslistnode->virusname)) {
                    skipListInsert(viruslistnode->citizens,citizen);
                    citizen->country = c->country;
                    return;
                }
                listnodetemp = listnodetemp->next;
            }
            // if this virus does not exist in this country, 
            // create a node for virus and its skip list
            countryVirusNode *newnode = calloc(1,sizeof(countryVirusNode));
            newnode->virusname = strdup(citizen->virusname);
            newnode->citizens = skipListCreate();
            skipListInsert(newnode->citizens, citizen);
            listpush(c->viruses,newnode);
            citizen->country = c->country;
            return;
        }
        temp=temp->next;
    }
}

void ListCountryDelete(List *list) 
{
    Listnode *nextCountryptr=NULL, *nextlistnode=NULL;
    countryNode *countrynode=NULL;
    countryVirusNode *viruslistnode = NULL;

    while(list->head!=NULL) {
        // save next country node
        nextCountryptr=list->head->next;
        countrynode = (countryNode *) list->head->item;

        while(countrynode->viruses->head!=NULL) {
            // save next node of the list with viruses for each country
            nextlistnode = (Listnode *) countrynode->viruses->head->next;

            viruslistnode = (countryVirusNode *)  countrynode->viruses->head->item;
            skipListDestroy(viruslistnode->citizens);
            free(viruslistnode->virusname);
            free(viruslistnode);
            free(countrynode->viruses->head);
            countrynode->viruses->head = nextlistnode;
        }
        free(countrynode->viruses);
        free(countrynode->country);
        free(countrynode);
        free(list->head);
        list->head=nextCountryptr;
    }
    free(list);
}






//////////////////////////     some functions for virus list     //////////////////
////////////////////    every virus has each own bloom filter and 2 skip lists    /////////////

virusList *virusListCreate()
{
    virusList *list=NULL;
    list = (virusList *) calloc(1,sizeof(virusList));
    list->head = NULL;
    list->size=0;
    return list;
}

void virusListInsert(virusList *list, char *virusname)
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
    new->bloom = bloomCreate(virusname, list->bloomsize);
    
    new->vaccinated_persons = skipListCreate();
    new->not_vaccinated_persons = skipListCreate();

    new->req_stats = listcreate();
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
    Listnode *tempnode;
    while(list->head!=NULL) {
        next = list->head->next;
        bloomDestroy(list->head->bloom);
        skipListDestroy(list->head->vaccinated_persons);
        skipListDestroy(list->head->not_vaccinated_persons);
        free(list->head->virusName);
        if(list->head->req_stats != NULL) {
            while(list->head->req_stats->head!=NULL) {
                tempnode = list->head->req_stats->head->next;
                req_stat *r_stat = list->head->req_stats->head->item;
                free(r_stat->date);
                free(r_stat);
                free(list->head->req_stats->head);
                list->head->req_stats->head = tempnode;
            }
            
        }
        free(list->head->req_stats);
        free(list->head);

        
        list->head = next;
    }
    if(list->input_dir!=NULL)
        free(list->input_dir);
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


// ~~~~~~~~~~~~~~~~~ parent virus list ~~~~~~~~~
void parentVirusListDestroy(List *list)
{
    Listnode *listnode;
    virusParentNode *vpnode;

    while(list->head!=NULL) {
        listnode = list->head->next;
        vpnode = (virusParentNode *) list->head->item;
        virusListDestroy(vpnode->vlist);
        free(vpnode->country);
        free(vpnode);
        free(list->head);
        list->head = listnode;
    }
    free(list);
}

