
#include "types.h"

void add_global_monitor();
int delete_global_monitor();
void add_stats_global(Record *rec);
void add_country_global_struct(char *country);
int add_request_global_struct(bool accepted, char *country);
int add_global_struct_request_parent(bool accepted);
int send_country_parent();