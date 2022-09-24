#include "types.h"

skipList *skipListCreate();
bool increaseHeight();
void skipListInsert(skipList *list,void *item);
void skipListSearch(skipList *list, char *citizenID);
bool skipListSearchbool(skipList *list, char *citizenID, char *date1, char *date2);
bool skipListSearchVACCINATENOW(skipList *list, char *citizenID);
void skipListDelete(skipList *list, char *key);
void skipListPrint(skipList *list);
void skipListDestroy(skipList *list);