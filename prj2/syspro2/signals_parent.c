
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>

#include "logs.h"
#include "msg.h"

//check when a child process got a signal
//SIGINT and SIGQUIT have the same type of treatment, so we need only 1 flag for both
static volatile sig_atomic_t sig_int_quit, sig_usr2, sig_chld;

// pid of usr2 sender
static pid_t sender_pid;

//set with signals that we want to block and unblock
static sigset_t blocked_set;

//functions to turn on flags to later handle these signals
static void sig_int_quit_handler(){
    sig_int_quit = 1;
}

static void sig_usr2_handler(){
    sig_usr2 = 1;
}
static void sig_chld_handler(){
    sig_chld = 1;
}

void signals_init() {
    sigset_t sigset;
    struct sigaction sigact;

    memset(&sigact,0,sizeof(struct sigaction));

    // we will use them as flags, so initialize to 0
    sig_int_quit = 0;
    sig_usr2 = 0;
    sig_chld = 0;

    sigfillset(&sigact.sa_mask);  // Block all other signals while handling one

    //sigact.sa_flags = SA_RESTART;
  
    sigact.sa_handler = sig_int_quit_handler;
    sigaction(SIGINT,  &sigact, NULL);
    sigaction(SIGQUIT, &sigact, NULL);
    sigact.sa_handler = sig_chld_handler;
    sigaction(SIGCHLD, &sigact, NULL);
    sigact.sa_handler = sig_usr2_handler;
    //sigact.sa_handler = sig_usr2_handler;
    sigaction(SIGUSR2, &sigact, NULL);
    

    sigemptyset(&blocked_set);  // Set signals blocked during a command process
    sigaddset(&blocked_set, SIGINT);
    sigaddset(&blocked_set, SIGQUIT);
    sigaddset(&blocked_set, SIGUSR2);

    /*
    sigemptyset(&sigset);
    sigprocmask(SIG_SETMASK, &sigset, NULL); // Unblock every signal 
    */
}

void sig_block(){
    sigprocmask(SIG_BLOCK, &blocked_set, NULL);
}

void sig_unblock(){
    sigprocmask(SIG_UNBLOCK, &blocked_set, NULL);
}

////&$&$&$&$////&$&$&$&$////&$&$&$&$////&$&$&$&$
void handle_signals(int update_num, int *sig_updates, pid_t *sig_chld_pid);

void sig_got() {
    if(sig_int_quit) {
        sig_int_quit = 0;
        handle_signals(2,NULL,NULL);
    }
    if(sig_usr2) {
        sig_usr2 = 0;
        handle_signals(1,NULL,NULL);
        //do nothing
    }
    if(sig_chld) {
        sig_chld = 0;
        pid_t pid = waitpid(-1, NULL, WNOHANG);
        handle_signals(3,NULL,&pid);
    }
    
}

// update num:   0 for initialize, 1 for handling usr2
void handle_signals(int update_num, int *sig_updates, pid_t *sig_chld_pid) {
    static int *sig_updates_ptr;
    static pid_t *sig_chld_pid_ptr;
    if(update_num==0) {
        sig_updates_ptr = sig_updates;
        sig_chld_pid_ptr = sig_chld_pid;
        (*sig_updates)=0;
    }
    else if(update_num==1){
        (*sig_updates_ptr)=1;
    }
    else if(update_num==2){
        (*sig_updates_ptr)=2;
    }
    else if(update_num==3){
        (*sig_updates_ptr)=3;
        (*sig_chld_pid_ptr)=(*sig_chld_pid);
    }
}


