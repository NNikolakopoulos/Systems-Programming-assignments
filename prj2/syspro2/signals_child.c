
#include <sys/types.h>
#include <sys/stat.h>
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
#include "global.h"
//check when a child process got a signal
//SIGINT and SIGQUIT have the same type of treatment, so we need only 1 flag for both
static volatile sig_atomic_t sig_int_quit, sig_usr1;

//set with signals that we want to block and unblock
static sigset_t blocked_set;

//functions to turn on flags to later handle these signals
static void sig_int_quit_handler(){
    sig_int_quit = 1;
}

static void sig_usr1_handler(){
    sig_usr1 = 1;
}

void signals_init() {
    sigset_t sigset;
    struct sigaction sigact;
    memset(&sigact,0,sizeof(struct sigaction));

    // we will use them as flags, so initialize to 0
    sig_int_quit = 0;
    sig_usr1 = 0;

    sigfillset(&sigact.sa_mask);  // Block all other signals while handling one
  
    sigact.sa_handler = sig_int_quit_handler;
    sigaction(SIGINT,  &sigact, NULL);
    sigaction(SIGQUIT, &sigact, NULL);
    sigact.sa_handler = sig_usr1_handler;
    sigaction(SIGUSR1, &sigact, NULL);

    sigemptyset(&blocked_set);  // Set signals blocked during a command process
    sigaddset(&blocked_set, SIGINT);
    sigaddset(&blocked_set, SIGQUIT);
    sigaddset(&blocked_set, SIGUSR1);

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

void handle_usr1();

void sig_got() {
    if(sig_int_quit) {
        write_in_log();
        sig_int_quit = 0;
    }
    if(sig_usr1) {
        handle_usr1();
        sig_usr1 = 0;
    }
}


void handle_usr1() {
    if (kill(getppid(), SIGUSR2) == -1) {
        perror("Error sending usr2 to parent"); 
        exit(1);
    }
    if(send_country_parent() == -1) {
        perror("Error while sending country to parent, while handling usr1");
        exit(1);
    }
}


