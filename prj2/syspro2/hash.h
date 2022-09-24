#include "types.h"

int hashfunction(char *K,int size);
HThash HTcreate();
unsigned int HTsize(HThash hash);
HThash insertItem(HThash hash, void *item, virusList *list, char *countryname);
HThash HTinsert(HThash hash,void *item, virusList *list, char *countryname);
HThash HTinsertAll(HThash hash, virusList *viruslist,char *fname);
HThash HTinsertfiles(HThash hash, virusList *viruslist,char *dir);
void HTprint(HThash hash);
void HTdestroy(HThash hash);
List *HTget_entry(HThash hash, char *citizenID);