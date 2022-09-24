#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include "list.h"
#include "slist.h"
#include "hash.h"
#include "bloom.h"
#include "global.h"


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

    hash.size=5000;
    //hash.size=32768; // 2^15
    hash.loadfactor=1.8;
    hash.count=0;
    hash.arr = (List **) calloc(hash.size,sizeof(List *));
    hash.countryList = NULL;
    hash.filesParsed = NULL;
    hash.not_changed = false;

    //initialize
    for(int i=0; i<hash.size; i++) 
        hash.arr[i] = listcreate();        
    hash.countryList = listcreate();
    hash.filesParsed = listcreate();

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

    if(!rec->vaccinated && rec->dateVaccinated!=NULL) {
        printf("Error inserting record: New record has date , but its NOT vaccinated\n");
        recordDelete(rec);
        return hash;
    }
    if(rec->vaccinated && rec->dateVaccinated==NULL) {
        printf("Error inserting record: New record does not have date , but its vaccinated\n");
        recordDelete(rec);
        return hash;
    }

    temp = hash.arr[index]->head; 
    if(temp == NULL)
    {  
        if(!virusListSearch(list,rec->virusname)) {// if this virus does not exists in virus list 
            virusListInsert(list,rec->virusname); // add it
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
                virusListInsert(list,rec->virusname); // add it
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

    vaccinatedSTR=(char *) malloc(5*sizeof(char));
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
        token = strtok(NULL," ");
        // if it has date , save it
        if(token!=NULL) {
            token[strlen(token)-1]='\0';
            char *daystr=NULL,*monthstr=NULL,*yearstr=NULL;
            daystr=strtok(token,"-");
            monthstr=strtok(NULL,"-");
            yearstr=strtok(NULL,"-");
            rec->dateVaccinated = (char *) calloc(11,sizeof(char));
            sprintf(rec->dateVaccinated,"%s-%s-%s",yearstr,monthstr,daystr);
        }
        else 
            rec->dateVaccinated = NULL;

        if( strcmp(vaccinatedSTR,"YES")==0  || strcmp(vaccinatedSTR,"YES\n")==0)
            rec->vaccinated = true; 
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

char *newpath(char *path1,char *path2)
{
    char *newpath = (char *) malloc( strlen(path1) + strlen(path2) + 3 );
    strcpy(newpath,path1);
    strcat(newpath,"/");
    strcat(newpath,path2);
    return newpath;
}

HThash HTinsertfiles(HThash hash, virusList *viruslist, char *dir) 
{
    DIR *dirp1, *dirp2, *tempdir;
    struct dirent *direntp1, *direntp2;
    char *newname1 = NULL, *newname2 = NULL, *newfname = NULL;
    struct stat statbuf;

    if((dirp1=opendir(dir)) == NULL) {
        fprintf(stderr,"cannot open %s\n",dir);
        exit(1);
    }   
    
    while( (direntp1 = readdir(dirp1)) != NULL) {
        // skip current dir and parent dir. 0 inode number means an error occured, so try again
        if(direntp1->d_ino == 0 || !strcmp(direntp1->d_name, ".") || !strcmp(direntp1->d_name, "..")) 
            continue;

        newname1 = newpath(dir,direntp1->d_name);   //update the path and look for this particular directory entry:   ./src/direntp1->d_name
        
        // we need to check if this direcotry entry is directory or regural file
        if( stat(newname1,&statbuf) == -1) {
            perror("Error: stat failed");
            exit(30);
        }
        // if  its regural file
        if( (statbuf.st_mode & S_IFMT) == S_IFREG) {
            if(listsearch(hash.filesParsed, direntp1->d_name, comparestr)) {
                hash.not_changed = false;
                continue;
            }
                
            newfname = strdup(direntp1->d_name); 
            listpush(hash.filesParsed, newfname);
            hash = HTinsertAll(hash,viruslist,newname1);
        }
        else {
            // if its directory find its files 
            dirp2=opendir(newname1);
            while( (direntp2 = readdir(dirp2)) != NULL) {
                if(direntp2->d_ino == 0 || !strcmp(direntp2->d_name, ".") || !strcmp(direntp2->d_name, "..")) 
                    continue;
                
                listprintf(hash.filesParsed);
                // if this file is already inserted in the hashtable, then continue with next one
                if(listsearch(hash.filesParsed, direntp2->d_name, comparestr)) {
                    hash.not_changed = false;
                    continue;
                }
                
                newfname = strdup(direntp2->d_name); 
                listpush(hash.filesParsed, newfname);
                newname2 = newpath(newname1,newfname);
                //printf("%s\n",newname2);
                hash = HTinsertAll(hash,viruslist,newname2);
                free(newname2);
            }
            closedir(dirp2);
        }
        free(newname1);
    }
    closedir(dirp1);
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
                printf("%s %s %s %s %d %s %s %s\n", rec->citizenID, rec->firstname, rec->lastname,  rec->country , rec->age, rec->virusname, (rec->vaccinated) ? "YES" : "NO", rec->dateVaccinated);
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
    listdeletefilesparsed(hash.filesParsed);
    free(hash.arr);

}

List *HTget_entry(HThash hash, char *citizenID) {
    int index=hashfunction(citizenID,hash.size); 
    return hash.arr[index];
}