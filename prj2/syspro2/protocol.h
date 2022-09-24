#include "types.h"

int protocol_commands_monitor(char *package, HThash hash, virusList *list, int writefd);
int protocol_commands_parent(char *package, int *pipefds, List *list, int monitor, char **args, List *clist);
int code_1(char *msg, HThash hash, virusList *list, int writefd);
int code_2(char *msg, unsigned long msg_length,HThash hash, virusList *list);
int code_3(char *msg, unsigned long msg_length, List *list,int monitor);
int code_4(char *msg, unsigned long msg_length,HThash *hash, virusList *list, int writefd);
int code_5(char *msg, unsigned long msg_length,HThash hash, virusList *list, int writefd);
int code_a(char *msg,unsigned long msg_length);
int code_6(char *msg, unsigned long msg_length, List *list, char **args);
int code_parent_0(char *msg, unsigned long msg_length);

char *copy_word_from_buf(char *msg);
char *last_months(char *date);
char *reverse_date(char *temp);