
#include "types.h"

unsigned long djb2(unsigned char *str);
unsigned long sdbm(unsigned char *str);
unsigned long hash_i(unsigned char *str, unsigned int i);
bloomf *bloomCreate(char *virus);
bool bloomSearch(bloomf *bloom, char *citizenid);
void bloomInsert(bloomf *bloom, Record *rec);
void bloomInsertAll(bloomf *bloom, HThash hash, char *virusname);
void bloomDestroy(bloomf *bloom);
