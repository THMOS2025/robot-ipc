#define _GNU_SOURCE    // for vscode to recognize variables

#include <time.h>
#include <fcntl.h> 
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdatomic.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include "robot_ipc_constant.h"
#include "host_variable.h"
#include "config.h"

#ifndef NDEBUG
/* the variable to indicate whether we should ignore the ctrl-C signal */
uint8_t _host_variable_should_wait = 0;
void (*_host_variable_next_handler)(int);
#endif

struct _s_host_variable {
    /* atomic_* is the newest feature in C11, providing a series of 
     * atomic operation at the aims of replacing mutex. To make use of 
     * this, all states should be compressed into a uint64 flags. 
     * In this algorithm, we have to track a 'target' pointer referring 
     * to the lastest (readable) buffer, and a lock_cnt for each buffer
     * to avoid conflict.
     *
     * [0 - 3] [3 - 7] ... [52 - 55]: represent the lock_cnt for each buffer
     * [56 - 63]: storing the 'target' pointer
     */
    atomic_uint_least64_t flags; 

    /* struct timespec contains a time_t and a long. On x64 linux systems, 
     * time_t is alias of int64_t and long is also int64_t. So, it is 
     * efficient to compress it into a int128_t, with simp/avx support.
     * To keep atomically ( which is important in multi-threading, we have 
     * to use the extension provided by GCC. If the platform doesn't support 
     * atomic int128, GCC just falls back to mutex lock for us. */
    atomic_uint_least64_t timestamp[SHM_BUFFER_CNT];

    /* below are the data field. Notice that the data field isn't fixed size
     * with a flexible array member, so this struct shouldn't be construct 
     * directly ( must be allocated manually ) */
    uint8_t data[];
};


#define FULL_SIZE(size) \
    (sizeof(struct _s_host_variable) + size * SHM_BUFFER_CNT)


static uint64_t inline
get_compressed_timestamp()
{
    struct timespec boot_ts; /* use boot time to avoid overflow */
    clock_gettime(CLOCK_BOOTTIME, &boot_ts);
    return ((uint64_t)boot_ts.tv_sec << 32ull) \
        | ((uint64_t)boot_ts.tv_nsec & 0x00000000FFFFFFFFull);
}
 

#ifndef NDEBUG
static void
__handler_wrap(int signum)
{
    if(_host_variable_should_wait) return;
    if(_host_variable_next_handler == SIG_IGN) return;
    else if(_host_variable_next_handler == SIG_DFL) {
        switch(signum) {
        case 1:
        case 2:
        case 9:
        case 13:
        case 14:
        case 15:
            exit(0);
        default:
            return;
        }
    }
    else 
        _host_variable_next_handler(signum);
}


static void
_set_handler_wrap()
{
    struct sigaction action;
    if (sigaction(SIGINT, NULL, &action) == -1) return;
    _host_variable_next_handler = action.sa_handler;
    sigemptyset(&action.sa_mask);
    action.sa_handler = __handler_wrap;
    action.sa_flags &= ~SA_SIGINFO;
    sigaction(SIGINT, &action, NULL);
}
#endif

host_variable link_host_variable(const char *name, const size_t size)
{

#ifndef NDEBUG
    /* we wrap the default handler for keyboard interrupt to 
     * prevent exit while memory copying which may cause dead
     * lock. */
    _set_handler_wrap();
#endif

    /* Try to open an existing and create if failed 
     *     0_CREAT | O_EXCL: create a new shared memory object
     *     O_RDWR: open for reading and writing
     *     shm_open: create or open a POSIX shared memory object
     *     0600: read and write permissions for the owner */
    const size_t full_size = FULL_SIZE(size);
    int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
    bool is_create = true;
    if(fd < 0) {
        if(errno == EEXIST) {
            // open the existing shared memory object
            fd = shm_open(name, O_RDWR, 0600);
            is_create = false;
        }
        if(fd < 0)   // other error cause cant open the shared memory object
            goto FAILED;
    }

    // Change the size of the shared memory object(file) to the required size
    if(ftruncate(fd, full_size) == -1)
        goto FAILED;
    
    // Map it into process's memory 
    host_variable p = (host_variable)mmap(
        NULL, 
        full_size, 
        PROT_READ | PROT_WRITE, 
        MAP_SHARED,
        fd, 
        0
    );
    if(p == MAP_FAILED) 
        goto FAILED;

    // Initialize the flags
    if(is_create) { 
        /* set flags to 0b11..1110000 */
        atomic_store(&p->flags, \
                (atomic_uint_least64_t) \
                ((((1ull << (14 - SHM_BUFFER_CNT)*4) - 1) \
                    << (SHM_BUFFER_CNT*4))));
        
        /* timestamp to 0b00..0000000 */
        for(uint8_t i = 0; i < SHM_BUFFER_CNT; ++i)
            atomic_store(&p->timestamp[i], (atomic_uint_least64_t)0);
    }

    close(fd); /* we can just close the file descripter after mmap */
    return p;

FAILED:
    // If we reach here, something went wrong, clean up
    if (fd != -1)
        close(fd);
    if ((void*)p != MAP_FAILED && p != NULL) 
        munmap(p, full_size);
    shm_unlink(name);
    return NULL;
}


int unlink_host_variable(host_variable p, const char *name, const size_t size)
{
    int ret = 0;
    ret |= munmap(p, FULL_SIZE(size));
   // ret |= shm_unlink(name);
    return ret;
}


int read_host_variable(host_variable p, void *buf, \
        const size_t size, const size_t op_size)
{
    int target;
    uint64_t flags, tmp, new_flags;

#ifndef NDEBUG
    _host_variable_should_wait = 1;
#endif

    /* add to the lock_cnt for the target buffer */
    flags = atomic_load(&p->flags);
    while(true) {
        target = (flags >> 56);
        tmp = ((flags >> (target * 4)) & 0xF) + 1;
        if(tmp == 0x0) /* overflood */
            return -1; /* lock is full */
        new_flags = ((flags & ~(0xF << (target*4))) | (tmp << (target*4)));
        /* atomic_compare_exchange_strong(*atomic, *expected, new) checks 
         * whether atomic variable equals to the expected value. If so, 
         * the atomic variable is set to new; otherwise, the expected value is
         * updated to the current value of the atomic value. This api is a 
         * low-level instruction of CPU, and it is GUARANTEE to be atomic for 
         * the whole compare-exchange process. */
        if(atomic_compare_exchange_strong(&p->flags, &flags, new_flags))
            break;
    }    
     
    /* copy the data */
    memcpy(buf, p->data + target * size, op_size);    

    /* reduce the lock_cnt for the target buffer. Be careful that we
     * shouldn't get the latest target here. */
    flags = atomic_load(&p->flags);
    while(true) {
        tmp = ((flags >> (target * 4)) & 0xF) - 1;
        if(tmp == 0xF) /* overflood */
            return -2; /* unknown error, maybe we can just set it to 0 here */
        new_flags = ((flags & ~(0xF << (target*4))) | (tmp << (target*4)));
        if(atomic_compare_exchange_strong(&p->flags, &flags, new_flags))
            break;
    }    

#ifndef NDEBUG
    _host_variable_should_wait = 0;
#endif

    return 0;
}


int write_host_variable(host_variable p, const void *data, \
        const size_t size, const size_t op_size)
{
    int target4; /* 4-times of the target buffer we're going to write to */
    int old_target; /* the current target buffer for reading */
    uint64_t flags, new_flags;
    uint64_t timestamp = get_compressed_timestamp(); /* time since boot */
    
#ifndef NDEBUG
    _host_variable_should_wait = 2;
#endif

    /* first acquire a free buffer */
    flags = atomic_load(&p->flags);
    while(true) {
        old_target = (flags >> 56ull);
        target4 = __builtin_ctzll( \
                ~(flags | (flags >> 1) | (flags >> 2) | (flags >> 3) \
                    | (0xFull << old_target*4)) \
                & 0x0011111111111111ull);
        if(target4 > SHM_BUFFER_CNT * 4)
            return -1; /* all buffers are full */
        new_flags = (flags | (0x1ull << target4));
        if(atomic_compare_exchange_strong(&p->flags, &flags, new_flags))
            break;
    }
    

    /* then memcopy */
    p->timestamp[target4>>2] = timestamp;
    memcpy(p->data + (target4>>2) * size, data, op_size);

    /* finally set the buffer to free */
    flags = atomic_load(&p->flags);
    while(true) {
        old_target = (flags >> 56ull);
        if(atomic_load(&p->timestamp[old_target]) >= timestamp) {
            /* release the block we just acquire, cause we are not lastest */
            __sync_fetch_and_and(&p->flags, ~(0xFull << target4));
            return 1;
        }
        /* construct the new_flags. Set the highest 56-63 bit to the new
         * target, and set the lock_cnt of the target buffer (the one
         * we just wrote to ) to 0. */
        new_flags = ((uint64_t)(target4>>2) << 56ull) \
                    | (flags & 0x00FFFFFFFFFFFFFFull & ~(0xFull << target4));
        if(atomic_compare_exchange_strong(&p->flags, &flags, new_flags))
            break;
    }
    
#ifndef NDEBUG
    _host_variable_should_wait = 0;
#endif

    return 0;
}


#undef TIMESPEC_TO_UINT64
#undef FULL_SIZE
