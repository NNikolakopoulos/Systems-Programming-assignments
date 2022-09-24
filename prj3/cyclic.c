#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <pthread.h>

#include "cyclic.h"

#define perror2(s,e) fprintf(stderr, "%s: %s\n", s, strerror(e))

pthread_cond_t cond_empty;
pthread_cond_t cond_full;

static pthread_mutex_t buf_mtx;

extern cyclicBuf *cb;

void mtx_cond_init() {
    pthread_cond_init(&cond_empty,NULL);
    pthread_cond_init(&cond_full,NULL);
    pthread_mutex_init( &buf_mtx,NULL);
}

void cond_signal_full() {
    pthread_cond_signal(&cond_full);
}

void cond_broadcast_empty() {
    pthread_cond_broadcast(&cond_empty);   
}

cyclicBuf *cyclicBuf_init(unsigned int cyclicBufferSize) {
    cyclicBuf *cb = calloc(1,sizeof(cyclicBuf));

    cb->size = cyclicBufferSize;
    cb->first = 0;
    cb->last = 0;
    cb->numElements = 0;
    cb->paths = malloc(cyclicBufferSize * sizeof(char *));
    for(unsigned int i=0; i<cyclicBufferSize; i++)
        cb->paths[i] = NULL;
    return cb;
}

void cyclicBuf_insert(cyclicBuf *cb, char *path) {
    pthread_mutex_lock(&buf_mtx);

    while(cb->size <= cb->numElements) { //if the buffer is full, wait to empty a cell
        int err = pthread_cond_wait(&cond_full,&buf_mtx);
        if(err!=0) {
            perror2("cond wait full",err);
        }
    }
    cb->paths[cb->last] = path;
    cb->last = (cb->last + 1) % cb->size;
    (cb->numElements)++;

    pthread_mutex_unlock(&buf_mtx);
}

char *cyclicBuf_pop(cyclicBuf *cb) {
    pthread_mutex_lock(&buf_mtx);

    while(cb->numElements <= 0) {//if the buffer is epmty, wait for at least one item to get inserted
        int err = pthread_cond_wait(&cond_empty,&buf_mtx);
        if(err!=0) {
            perror2("cond wait full",err);
        }
    }
    char *path;
    path = cb->paths[cb->first];
    cb->paths[cb->first] = NULL;
    cb->first = (cb->first + 1) % cb->size;
    (cb->numElements)--;

    pthread_mutex_unlock(&buf_mtx);

    return path;
}

void cyclicBuf_delete(cyclicBuf *cb) {
    free(cb->paths);
    free(cb);
}

void mtx_cond_destroy(unsigned int numThreads) {
    for (int i = 0; i < numThreads; ++i) {
        //printf("y1\n");
        cyclicBuf_insert(cb,NULL);
        //printf("y2\n");
        pthread_cond_broadcast(&cond_empty);
        //printf("y3\n");
        //pthread_cond_signal(&cond_full);
    }

    pthread_cond_destroy(&cond_full); 
    pthread_cond_destroy(&cond_empty);
    pthread_mutex_destroy(&buf_mtx);

}