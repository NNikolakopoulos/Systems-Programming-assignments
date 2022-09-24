#include "types.h"

List *listcreate();
void listpush(List *l,void *data);
void recordDelete(void *rec);
void listdelete(List *l);
void listpop(List *l);
void listprint(List *l); 

void ListCountryInsert(List *list, Record *citizen, char *countryname);
void ListCountryIndex(List *list,Record *citizen,char *countryname);
void ListCountryDelete(List *list);

virusList *virusListCreate();
void virusListInsert(virusList *list, char *virusname, HThash hash);
virusListNode *virusListSearch(virusList *list, char *virusname);
void virusListDestroy(virusList *list);
void virusListPrint(virusList *list);