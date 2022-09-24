
#include "signal.h"

static void sig_int_quit_handler();
static void sig_usr2_handler(int sig, siginfo_t *info, void *ucontext);
static void sig_chld_handler();
void signals_init();
void sig_block();
void sig_unblock();
void sig_got();

void handle_signals(int update_num, int *sig_updates, pid_t *sig_chld_pid);