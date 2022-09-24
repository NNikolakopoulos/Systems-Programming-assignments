#include "types.h"

void mtx_cond_init();
void cond_signal_full();
void cond_broadcast_empty();
cyclicBuf *cyclicBuf_init(unsigned int cyclicBufferSize);
void cyclicBuf_insert(cyclicBuf *cb, char *path);
char *cyclicBuf_pop(cyclicBuf *cb);
void cyclicBuf_delete(cyclicBuf *cb);
void mtx_cond_destroy(unsigned int numThreads);