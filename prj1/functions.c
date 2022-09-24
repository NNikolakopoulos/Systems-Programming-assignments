#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "list.h"
#include "slist.h"
#include "hash.h"
#include "bloom.h"




void vaccineStatusBloom(virusList *list, char *citizenid, char *virusname)
{
    virusListNode *temp=NULL;

    if(list==NULL) {
        printf("Error: vaccineStatusBloom \"virusList\" is NULL\n");
        exit(1);
    }
    if(citizenid==NULL || citizenid[0]=='-' || strlen(citizenid)>4) {
        printf("Error: \"vaccineStatusAll\" wrong citizenid\n");
        exit(2);
    }
    if(virusname==NULL) {
        printf("Error: \"vaccineStatusAll\" insert a virus name\n");
        exit(2);
    }

    temp=list->head;
    while(temp!=NULL) {
        if(strcmp(temp->virusName, virusname)==0) {
            if(bloomSearch(temp->bloom, citizenid)) {
                printf("MAYBE\n");
                return;
            }
            else
                break;
        }
        temp=temp->next;
    }
    printf("NOT VACCINATED\n");
}

void vaccineStatus(virusList *list, char *citizenid, char *virusname) 
{
    virusListNode *temp=NULL;

    if(list==NULL) {
        printf("Error: vaccineStatus \"virusList\" is NULL\n");
        exit(1);
    }
    if(citizenid==NULL || citizenid[0]=='-' || strlen(citizenid)>4) {
        printf("Error: \"vaccineStatusAll\" wrong citizenid\n");
        exit(2);
    }
    if(virusname==NULL) {
        printf("Error: \"vaccineStatusAll\" insert a virus name\n");
        exit(2);
    }

    temp=list->head;
    while(temp!=NULL) {
        if(!strcmp(temp->virusName, virusname)) {
            skipListSearch(temp->vaccinated_persons, citizenid);
            return;
        }
        temp=temp->next;
    }
    printf("NOT VACCINATED\n");
}


void vaccineStatusAll(virusList *list, char *citizenid) 
{
    virusListNode *temp=NULL;

    if(list==NULL) {
        printf("Error: vaccineStatusAll \"virusList\" is NULL\n");
        exit(1);
    }
    if(citizenid==NULL || citizenid[0]=='-' || strlen(citizenid)>4) {
        printf("Error: \"vaccineStatusAll\" wrong citizenid\n");
        exit(2);
    }

    temp=list->head;
    while(temp!=NULL) {
        printf("%s ",temp->virusName);
        if(!skipListSearchbool(temp->vaccinated_persons, citizenid, "0000-00-00", "3000-00-00")) 
            skipListSearch(temp->not_vaccinated_persons, citizenid); 
        else {
            skipListSearch(temp->vaccinated_persons, citizenid);
        }
        temp=temp->next;
    }
}

void populationStatus(HThash hash, virusList *list, char *virusname, char *date1, char *date2, char *country)
{
    virusListNode *vnode=NULL;
    countryNode *cnode=NULL;
    Listnode *lnode=NULL, *citizenNode=NULL;
    Record *rec=NULL;
    unsigned long int count, totalcount;

    if(list==NULL) {
        printf("Error: In \"populationStatus\" Virus list is empty\n");
        exit(1);
    }
    if( (date2==NULL && date1!=NULL) || (date1==NULL && date2!=NULL)) {
        printf("Error: Only 1 date given\n");
        exit(1);
    }
    if(date1==NULL && date2==NULL) {
        date1=strdup("0000-00-00");
        date2=strdup("3000-00-00");
    }
    //for every country
    lnode = hash.countryList->head;
    while(lnode!=NULL) {
        cnode = (countryNode *) lnode->item;
        // if we are NOT looking for a certain country(==NULL), then continue the procedure for all countries 
        // or if we are looking for a certain country, then print the statistics only for this country
        if( country==NULL || strcmp(cnode->country, country)==0) {
            totalcount=0;
            count=0;
            //now we need to check every citizen of this country
            citizenNode = cnode->citizens->head;
            while(citizenNode!=NULL) {
                rec = (Record *) citizenNode->item;
                if(rec->vaccinated && strcmp(rec->virusname,virusname)==0 && (strcmp(date1, rec->dateVaccinated)<0) && (strcmp(date2, rec->dateVaccinated)>0))
                    count++;
                if(strcmp(rec->virusname,virusname)==0)
                    totalcount++;
                
                citizenNode=citizenNode->next;
            }
        }
        lnode=lnode->next;
        if(country!=NULL && strcmp(country, cnode->country)!=0)
            continue;
        printf("%s %ld %0.2f%%\n",(country==NULL)?cnode->country:country, count, (float) count*100.0/(float)(totalcount));
    }
    free(date1); free(date2);
}


void popStatusByAge(HThash hash, virusList *list, char *virusname, char *date1, char *date2, char *country)
{
    virusListNode *vnode=NULL;
    countryNode *cnode=NULL;
    Listnode *lnode=NULL, *citizenNode=NULL;
    Record *rec=NULL;
    unsigned long int totalcount, *count=NULL;

    count = (unsigned long int *) calloc(4,sizeof(unsigned long int));

    if(list==NULL) {
        printf("Error: In \"populationStatus\" Virus list is empty\n");
        exit(1);
    }
    if( (date2==NULL && date1!=NULL) || (date1==NULL && date2!=NULL)) {
        printf("Error: Only 1 date given\n");
        exit(1);
    }
    if(date1==NULL && date2==NULL) {
        date1=strdup("0000-00-00");
        date2=strdup("3000-00-00");
    }
    //for every country
    lnode = hash.countryList->head;
    while(lnode!=NULL) {
        cnode = (countryNode *) lnode->item;
        // if we are NOT looking for a certain country(==NULL), then continue the procedure for all countries 
        // or if we are looking for a certain country, then print the statistics only for this country
        if( country==NULL || !strcmp(cnode->country, country)) {
            totalcount=0;
            count[0]=0;count[1]=0;count[2]=0;count[3]=0;
            //now we need to check every citizen of this country
            citizenNode = cnode->citizens->head;
            while(citizenNode!=NULL) {
                rec = (Record *) citizenNode->item;
                if(rec->vaccinated && strcmp(rec->virusname,virusname)==0 && (strcmp(date1, rec->dateVaccinated)<0) && (strcmp(date2, rec->dateVaccinated)>0)) {
                    if(rec->age<20)
                        count[0]++;
                    if(rec->age<40 && rec->age>=20)
                        count[1]++;
                    if(rec->age<60 && rec->age>=40)
                        count[2]++;
                    if(rec->age>=60)
                        count[3]++;
                }
                if(strcmp(rec->virusname,virusname)==0)
                    totalcount++;
                
                citizenNode=citizenNode->next;
            }
        }
        lnode=lnode->next;
        if(country!=NULL && strcmp(country, cnode->country)!=0)
            continue;
        printf("\n%s\n",(country==NULL)?cnode->country:country);
        printf("0-20 %ld %0.2f%%\n", count[0], (float) count[0]*100.0/(float)(totalcount));
        printf("20-40 %ld %0.2f%%\n", count[1], (float) count[1]*100.0/(float)(totalcount));
        printf("40-60 %ld %0.2f%%\n", count[2], (float) count[2]*100.0/(float)(totalcount));
        printf("60+ %ld %0.2f%%\n", count[3], (float) count[3]*100.0/(float)(totalcount));
    }
    free(count); free(date1); free(date2);
}

void insertCitizenRecord(HThash hash, virusList *list, char *citizenid, char *firstname, char *lastname, char *country, int age, char *virusname, char *vaccinated, char *date) 
{
    Record *rec=NULL;

    rec = (Record *) calloc(1,sizeof(Record));
    rec->citizenID = (char *) calloc(5,sizeof(char));
    rec->firstname = (char *) calloc(13,sizeof(char));
    rec->lastname = (char *) calloc(13,sizeof(char));
    rec->virusname = (char *) calloc(10,sizeof(char));

    strcpy(rec->citizenID,citizenid);
    strcpy(rec->firstname,firstname);
    strcpy(rec->lastname,lastname);
    rec->age=age;
    strcpy(rec->virusname,virusname);
    if(strcmp(vaccinated, "YES")==0) {
        rec->vaccinated=true;
        rec->dateVaccinated = (char *) calloc(11,sizeof(char));
        strcpy(rec->dateVaccinated, date);
    }
    else
        rec->vaccinated=false;

    HTinsert(hash, rec, list, country);


}

void vaccinateNow(HThash hash, virusList *list, char *citizenid, char *fname, char *lname, char *country, int age, char *virusname)
{
    virusListNode *temp=NULL;

    temp = list->head;
    while(temp!=NULL) {
        if(!strcmp(virusname, temp->virusName)) {
            // if this person is already vaccinated, then print error
            if(!skipListSearchVACCINATENOW(temp->vaccinated_persons, citizenid)) {
                //if it isnt vaccinated, then insert 
                char *date = (char *) calloc(10, sizeof(char));
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                sprintf(date,"%d-%d-%d",tm.tm_year + 1900,tm.tm_mon + 1, tm.tm_mday);
                insertCitizenRecord(hash, list, citizenid, fname, lname, country,  age, virusname, "YES", date);  
                skipListDelete(temp->not_vaccinated_persons, citizenid);
            }
        }
        temp=temp->next;
    }

}

void listnonVaccinatedPersons(virusList *list, char *virusname) 
{
    virusListNode *temp=NULL;

    temp = list->head;
    while(temp!=NULL) {
        if(!strcmp(virusname, temp->virusName)) {
            printf("List of people not vaccinated in %s:\n",virusname);
            skipListPrint(temp->not_vaccinated_persons);
            return;
        }
        temp=temp->next;
    }
    printf("Please give a valid virus\n");
}   