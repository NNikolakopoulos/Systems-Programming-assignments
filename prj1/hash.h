#include "types.h"

int hashfunction(char *K,int size);
HThash HTcreate();
unsigned int HTsize(HThash hash);
HThash insertItem(HThash hash, void *item, virusList *list, char *countryname);
HThash HTinsert(HThash hash,void *item, virusList *list, char *countryname);
HThash HTinsertAll(HThash hash, virusList *viruslist,char *fname);
void HTprint(HThash hash);
void HTdestroy(HThash hash);