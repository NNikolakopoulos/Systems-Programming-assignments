#include "types.h"

char **command_prompt();
unsigned int get_rand_port(int *ports, int numMonitors);
char *get_exec_args(char *exec_args,unsigned int port,long int bloomsize,long int socketBufferSize, int cyclicBufferSize, int numThreads, char * input_dir, int numMonitors, int iMonitor);
virusListNode *get_country_node(List *list, int *monitor_i, char *country, char *virusName);
void free_args(char ** args);
int clist_add_request(List *list,bool accepted,char *request_date, char *country, char *virusName);
virusListNode *get_vnode(List *list, char *country, char *virusname);
char *get_stats_all_countries(List *list,char *reversedDate1,char *reversedDate2,char *virusname);
int get_country_node_index(List *list, int *monitor_i,char *country);
char *get_input_dir(char *path);
