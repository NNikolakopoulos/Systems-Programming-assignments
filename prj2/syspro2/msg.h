

int safe_read(int fd,char* buf,size_t size, bool signals);
int safe_write(int fd,char* buf,size_t size);
int pipe_write(int fd, char *buf,long int bufsiz, char protocol);
char * pipe_read(int fd);
char *pipe_read_sig(int fd);
char get_protocol(char *buf);
long int get_package_length(char *buf);
char *get_msg(char *buf);
int initialize_monitors(int *fds, char *input_dir, int numMonitors);
int initialize_monitors0(int *fds, unsigned long bloomsize, unsigned long bufsize, char *input_dir, int numMonitors);
int parentListInit(List *list, char *input_dir, unsigned long bloomsize,int numMonitors);
int ready_for_bloom(int *fds, int numMonitors);
int ready_to_exit(int *fds, int numMonitors);
int send_travelRequest(int *fds,char **args, int monitor_i);
int send_code_8(pid_t sender_pid);
int get_travel_stats(char **args,List *list);
int send_request_response(int *fds, List *list, int numMonitors, char *date, char *country, char *virusname, bool accepted);
int send_search_status(int *pipesfds,char **args, int numMonitors);
int quit(pid_t *pids,int numMonitors);
int spawn_monitor( int *pipesfds, List *list,pid_t sig_chld_pid, char *input_dir, int numMonitors, pid_t *pids, unsigned long bloomsize, unsigned long pipe_buf_size);