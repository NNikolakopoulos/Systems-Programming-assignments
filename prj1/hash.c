#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>


#include "list.h"
#include "slist.h"
#include "hash.h"
#include "bloom.h"


int hashfunction(char *K,int size)
{
 int h=0, a=33;
    for (; *K!='\0'; K++)
        h=(a*h + *K) % size;
    return h;
} 


//create a new hash table
HThash HTcreate()
{
    HThash hash;

    hash.size=50000;
    //hash.size=32768; // 2^15
    hash.loadfactor=1.8;
    hash.count=0;
    hash.arr = (List **) calloc(hash.size,sizeof(List *));
    hash.countryList = NULL;

    //initialize
    for(int i=0; i<hash.size; i++) 
        hash.arr[i] = listcreate();        
    hash.countryList = listcreate();

    return hash;
}



// this function will make more simple the HTinsert function
HThash insertItem(HThash hash, void *item, virusList *list, char *countryname)
{
    Listnode *temp=NULL;
    virusListNode *viruslistnode=NULL;
    bool flag=true;
    Record *temprec=NULL, *rec=(Record *) item;
    int index=hashfunction(rec->citizenID,hash.size);   

    temp = hash.arr[index]->head; 
    if(temp == NULL)
    {   
        if(!virusListSearch(list,rec->virusname)) {// if this virus does not exists in virus list 
            virusListInsert(list,rec->virusname,hash); // add it
            //printf("%s\n",(*viruslist)->virusName);
        }
        hash.count++;
        listpush(hash.arr[index],item);  
        viruslistnode=virusListSearch(list, rec->virusname);
        if(rec->vaccinated) {
            skipListInsert(viruslistnode->vaccinated_persons, item);
            bloomInsert(viruslistnode->bloom,item);
            
        }
        else
            skipListInsert(viruslistnode->not_vaccinated_persons, item);
        ListCountryInsert(hash.countryList,item,countryname);
        return hash;
    }
    else
    {
        while( temp != NULL)  
        {
            temprec = (Record *) temp->item;
            // if this person has already done the vaccine for this particular virus, then don't insert into hashtable
            // if same id 
            if(!rec->vaccinated && rec->dateVaccinated!=NULL) {
                printf("Error inserting record: New record has date , but its NOT vaccinated\n");
                recordDelete(rec);
                return hash;
            }
            if(!strcmp(temprec->citizenID, rec->citizenID)) {
                flag=false;
                // with at least 1 more different characteristic (firstname or lastname or age or country or age)
                if(strcmp(temprec->firstname, rec->firstname)!=0 || strcmp(temprec->lastname, rec->lastname)!=0 || strcmp(temprec->country, countryname)!=0 || temprec->age!=rec->age ) {
                    printf("Error inserting record: same ID, 1 at least different characteristic\n");
                    recordDelete(rec);
                    return hash;
                }
                //if exists with same characteristic and virus name
                if(!strcmp(temprec->firstname, rec->firstname) && !strcmp(temprec->lastname, rec->lastname) && !strcmp(temprec->country, countryname) && temprec->age==rec->age && !strcmp(temprec->virusname, rec->virusname)) {
                    //if both records are vaccinated , but new record doesnt have any date
                    if(temprec->vaccinated && rec->vaccinated && rec->dateVaccinated==NULL) {
                        printf("Error inserting record: same ID, same characteristics, same virusname, both vaccinated, but new record does not have date\n");
                        recordDelete(rec);
                        return hash;
                    }
                    //if both records not vaccinated
                    if(!temprec->vaccinated && !rec->vaccinated) {
                        printf("Error inserting record: same ID, same characteristics, same virusname, both not-vaccinated\n");
                        recordDelete(rec);
                        return hash;
                    }
                    // if one record in database is vaccinated and the new record is not-vaccinated
                    if( temprec->vaccinated && !rec->vaccinated) {
                        printf("Error inserting record: same ID, same characteristics, same virusname, but one record is vaccinated and the other not\n");
                        recordDelete(rec);
                        return hash;
                    }/*
                    if( !temprec->vaccinated && rec->vaccinated) {
                        printf("Error inserting record: same ID, same characteristics, same virusname, but one record is vaccinated and the other not\n");
                        recordDelete(rec);
                        return hash;
                    }*/
                    // if both vaccinated (but have different date of vaccination )
                    if( temprec->vaccinated && rec->vaccinated) {// && strcmp(temprec->dateVaccinated, rec->dateVaccinated)!=0) {
                        printf("ERROR: CITIZEN %s ALREADY VACCINATED ON %s\n",temprec->citizenID, temprec->dateVaccinated);
                        recordDelete(rec);
                        return hash;
                    }

                }

            }
            
            temp=temp->next;
        }   
            if(!virusListSearch(list,rec->virusname)) {// if this virus does not exists in virus list 
                virusListInsert(list,rec->virusname,hash); // add it
            }
            hash.count++;
            listpush(hash.arr[index],item);
            viruslistnode=virusListSearch(list, rec->virusname);
            if(rec->vaccinated) {
                skipListInsert(viruslistnode->vaccinated_persons, item);
                bloomInsert(viruslistnode->bloom,item);
            }
            else
                skipListInsert(viruslistnode->not_vaccinated_persons, item);

            // if this country is NOT already inserted in the country list, then insert it
            if(flag)
                ListCountryInsert(hash.countryList,item,countryname);
            else
                ListCountryIndex(hash.countryList,item,countryname); // else use pointer to that name
        return hash;
    }         
}
HThash HTinsert(HThash hash,void *item,virusList *list, char *countryname)
{
    HThash newhash;
    Listnode *temp=NULL;
    
    Record *rec=NULL;
    rec = (Record *) item;
    int i=hashfunction(rec->citizenID,hash.size); 

    // ~~~~~~~~  SKIP  THIS CODE, ITS FOR DYNAMIC REHASHING  ~~~~~~~~~~~~~~~

    //if( ((double) hash.count / (double) hash.size) > hash.loadfactor) // check if rehashing is needed 
    if(0)
    {
        newhash.size=hash.size << 1 ; //size of new hash table is 2 * old hash table size
        newhash.arr = (List **) calloc(newhash.size,sizeof(List*)); //create new hash table
        newhash.loadfactor = hash.loadfactor;
        for(i=0 ; i < hash.size; i++) 
        {   
            temp=hash.arr[i]->head;
            if(temp != NULL) //if it isnt empty do :
            {   
                temp=temp->next;
                while(temp != NULL) 
                {
                    newhash=insertItem(newhash,temp->item,list,countryname); //copy the nodes to the new hash table
                    if(temp->next==NULL)
                        break;
                    temp=temp->next; 
                }
            }
        }
        HTdestroy(hash);
        // after we create a new,bigger table
        // we insert the item we wanted from the beginning
        newhash=insertItem(newhash,item,list,countryname); 
        return newhash;
    } 
    else
    {
        // ~~~~~~~ CONTINUE HERE ~~~~~~~~~~
        
        // if no rehashing is needed, just insert the item
        hash=insertItem(hash,item,list,countryname); 
        return hash;
    }
}

HThash HTinsertAll(HThash hash, virusList *viruslist,char *fname)
{
    Listnode *temp=NULL;
    FILE *fp = fopen(fname,"r");
    Record *rec=NULL;

    if(fp==NULL) {
        perror("ERROR: file not opened");
        exit(1);
    }

    size_t n = 0;
    char *vaccinatedSTR = NULL, *ageSTR = NULL, *token = NULL, *line = NULL, *countryname = NULL;

    vaccinatedSTR=(char *) malloc(4*sizeof(char));
    ageSTR=(char *) malloc(4*sizeof(char));
    countryname = (char *) malloc(21*sizeof(char));

    while( getline(&line, &n, fp) != -1)  {
        rec = (Record *) calloc(1,sizeof(Record));
        rec->citizenID = (char *) calloc(5,sizeof(char));
        rec->firstname = (char *) calloc(13,sizeof(char));
        rec->lastname = (char *) calloc(13,sizeof(char));
        rec->virusname = (char *) calloc(10,sizeof(char));
        

        

        token = strtok(line," ");
        strcpy(rec->citizenID,token);
        token = strtok(NULL," ");
        strcpy(rec->firstname,token);
        token = strtok(NULL," ");
        strcpy(rec->lastname,token);
        token = strtok(NULL," ");
        strcpy(countryname,token);
        token = strtok(NULL," ");
        strcpy(ageSTR,token);
        rec->age=atoi(ageSTR);
        token = strtok(NULL," ");
        strcpy(rec->virusname,token);
        token = strtok(NULL," ");
        strcpy(vaccinatedSTR,token);


        if( strcmp(vaccinatedSTR,"YES")==0 ) {
            //if this person has been vaccinated, scan the date
            rec->vaccinated = true;

            token = strtok(NULL," ");
            token[strlen(token)-1]='\0';

            char *daystr=NULL,*monthstr=NULL,*yearstr=NULL;
            daystr=strtok(token,"-");
            monthstr=strtok(NULL,"-");
            yearstr=strtok(NULL,"-");
            rec->dateVaccinated = (char *) calloc(11,sizeof(char));
            sprintf(rec->dateVaccinated,"%s-%s-%s",yearstr,monthstr,daystr);
        }
        else
            rec->vaccinated = false;

        
        hash=HTinsert(hash,rec,viruslist,countryname);
    }
    if(fclose(fp) == EOF) {
        perror("ERROR: file not closed");
        exit(2);
    }

            
    free(countryname);
    free(vaccinatedSTR);
    free(ageSTR);
    free(line);


    return hash;
}
void HTprint(HThash hash)
{
    Record *rec;
    int i;
    printf(" size is %d\n", hash.size);
    Listnode *temp;
    for(i=0; i<hash.size;i++)
    {
        temp=hash.arr[i]->head;
        if(temp!=NULL)
        {
            while(temp!=NULL)
            {
                rec = (Record *) temp->item;
                printf("%s %s %s %s %d %s %s %s\n", rec->citizenID, rec->firstname, rec->lastname,  rec->country , rec->age, rec->virusname, !(rec->vaccinated) ? "YES" : "NO", rec->dateVaccinated);
                if(temp->next==NULL)
                    break;
                temp=temp->next;
            }
        }
    }
}


void HTdestroy(HThash hash)
{
    int i;
    List *temp;

    for(i=0; i < hash.size; i++)
    {
        temp=hash.arr[i];
        listdelete(hash.arr[i]);
    }
    ListCountryDelete(hash.countryList);
    free(hash.arr);

}