#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>


#include "list.h"
#include "hash.h"
#include "bloom.h"

#define BLOOM_SIZE_BITS 800000

unsigned long djb2(unsigned char *str) {
	unsigned long hash = 5381;
	int c; 
	while (c = *str++) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	return hash;
}


unsigned long sdbm(unsigned char *str) {
	unsigned long hash = 0;
	int c;

	while (c = *str++) {
		hash = c + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}

unsigned long hash_i(unsigned char *str, unsigned int i) {
	return djb2(str) + i*sdbm(str) + i*i;
}

bloomf *bloomCreate(char *virus, unsigned long bloomsize)
{
    bloomf *bloom = (bloomf *) calloc(1,sizeof(bloomf));
    //bloom->size = BLOOM_SIZE_BITS/8; 
    bloom->size = bloomsize;
    bloom->arr = (uint8_t *) calloc(bloom->size+1, sizeof(uint8_t)); 
    bloom->hashfunctions = 16; // we have 16 hash functions
    bloom->virusName = strdup(virus);

    return bloom;
}

void bloomInsert(bloomf *bloom, Record *rec)
{
    unsigned long i, j;
    uint8_t mask=1, shift;

    for(int k=0; k<bloom->hashfunctions; k++) {
        i=(hash_i(rec->citizenID, k) % bloom->size);  //   *i-th* bit of the bloom filter 
        // but the indexes in bloom filter array are sizeof(char),
        // so we need some simple arithmetic to convert:  bit index -> char index
        j=i/8;  
        shift=i%8;  // and save the position of bit 

        bloom->arr[j] = (bloom->arr[j] | (mask << shift));   
    }
}

bool bloomSearch(bloomf *bloom, char *citizenid)
{
    unsigned long i, j;
    uint8_t shift, mask=1;

    for(int k=0; k<bloom->hashfunctions; k++) {
        i=(hash_i(citizenid, k) % bloom->size);
        j=i/8;  
        shift=i%8;
        if( ((bloom->arr[j] & (mask << shift)) == 0) ) 
            return false;
    }
    return true;
}

void bloomInsertAll(bloomf *bloom, HThash hash, char *virusname)
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
                    bloomInsert(bloom,rec);
                temp=temp->next;
            }
        }
    }

}

void bloomDestroy(bloomf *bloom) 
{
    free(bloom->arr);
    free(bloom->virusName);      
    free(bloom);
}
