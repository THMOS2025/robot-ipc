/* super_variable.h
 */

#ifndef _H_SUPER_VARIABLE

#define _GNU_SOURCE   // for pthread_mutex_consistent

#include <time.h>
#include <fcntl.h> 
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include "error_code.h"

#define CIRCLE_QUEUE_LENGTH 4  // must be the power of 2 

struct _s_super_variable {
    pthread_mutex_t mtx;
    uint8_t qhead, qtail;
    struct timespec timestamp;
    struct timespec b_timestamp[CIRCLE_QUEUE_LENGTH];
    uint8_t data[];
};

typedef struct _s_super_variable* super_variable;

super_variable link_super_variable(const char *name, size_t size);   // create or link to an existing super variable
int read_super_variable(super_variable p, void *buf, size_t size, struct timespec *out_timestamp);
int write_super_variable(super_variable p, void *data, size_t size, struct timespec *out_timestamp);
void unlink_super_variable(super_variable p, const char *name, size_t size);

#endif
