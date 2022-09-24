#include "types.h"

List *listcreate();
void listpush(List *l,void *data);
void recordDelete(void *rec);
void listdelete(List *l);
void listdeletefilesparsed(List *l);
void listpop(List *l);
void listprint(List *l); 
int comparestr(void *str1, void *str2);
void listprintstr(List *l);
bool listsearch(List *l, void *item, int compare(void *, void *));
void listprintf(List *list);


void ListCountryInsert(List *list, Record *citizen, char *countryname);
void ListCountryIndex(List *list,Record *citizen,char *countryname);
void ListCountryDelete(List *list);

virusList *virusListCreate();
void virusListInsert(virusList *list, char *virusname);
virusListNode *virusListSearch(virusList *list, char *virusname);
void virusListDestroy(virusList *list);
void virusListPrint(virusList *list);

void parentVirusListDestroy(List *list);