#include "super_variable.h"

static inline int acquire_meta_lock(super_variable p)
{
    int result = pthread_mutex_lock(&p->mtx);  // lock the mutex
    // If the mutex is robust, it may return EOWNERDEAD if the previous owner died
    if (result == EOWNERDEAD) {
        // The mutex is now locked, but the state needs to be reset
        // After this, it's safe to use the mutex again
        pthread_mutex_consistent(&p->mtx);
    } 
    else if (result != 0) {
        return result;
    }
    return 0;
}


static inline int release_meta_lock(super_variable p)
{
    pthread_mutex_unlock(&p->mtx);
    return 0;
}


static inline size_t get_full_size(size_t size)
{
    return sizeof(struct _s_super_variable) + size * CIRCLE_QUEUE_LENGTH;
}


static inline bool is_later_than(const struct timespec a, const struct timespec b)
{
    return a.tv_sec > b.tv_sec || (a.tv_sec == b.tv_sec && a.tv_nsec > b.tv_nsec);
}


super_variable link_super_variable(const char *name, size_t size)
{
    // Try to open an existing and create if failed 
    //     0_CREAT | O_EXCL: create a new shared memory object
    //     O_RDWR: open for reading and writing
    //     shm_open: create or open a POSIX shared memory object
    //     0600: read and write permissions for the owner
    int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
    bool is_create = true;
    if(fd == -1) {
        if(errno == EEXIST) {
            // open the existing shared memory object
            fd = shm_open(name, O_RDWR, 0600);
            is_create = false;
        }
        if(fd == -1)   // other error cause cant open the shared memory object
            goto END;
    }

    // Change the size of the shared memory object(file) to the required size
    if(ftruncate(fd, get_full_size(size)) == -1)
        goto END;
    
    // Map it into process's memory 
    super_variable p = (super_variable)mmap(
        NULL, 
        get_full_size(size), 
        PROT_READ | PROT_WRITE, 
        MAP_SHARED, 
        fd, 
        0
    );
    if(p == MAP_FAILED) 
        goto END;
    
    // Initialize the mutex if created 
    if(is_create) { 
        // Prepare for the attribute of the mutex lock
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        /* Robust mutex can dealing with process being killed */
        pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
        pthread_mutex_init(&p->mtx, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    return p;

END:
    // If we reach here, something went wrong, clean up
    if (fd != -1)
        close(fd);
    if ((void*)p != MAP_FAILED && p != NULL) 
        munmap(p, get_full_size(size));
    shm_unlink(name);
    return NULL;
}


void unlink_super_variable(super_variable p, const char *name, size_t size)
{
    munmap(p, get_full_size(size));
    shm_unlink(name);
}


int read_super_variable(super_variable p, void *buf, size_t size)
{
    int err_code = 0;
    // Modify to the meta block should be very fast 
    err_code = acquire_meta_lock(p);
    if(err_code) 
        return err_code;

    uint8_t qh = p->qhead, qt = p->qtail;
    // Compare the timestamp to find the latest data
    while(qh != qt && memcmp(&p->b_timestamp[qh], &p->timestamp, \
                sizeof(struct timespec))){
        qh = ((qh + 1) & (CIRCLE_QUEUE_LENGTH - 1)); 
    }

    p->qhead = qh; p->qtail = qt; 
    release_meta_lock(p);
    
    if(qh == qt) 
        return -1;  // Nothing to read

    // mem copying may be costly
    memcpy(buf, p->data + qh * size, size);
    
    return 0;
}


int write_super_variable(super_variable p, void *data, size_t size)
{
    int err_code = 0;
    uint8_t tmp;
    struct timespec ts;
    
    clock_gettime(CLOCK_REALTIME, &ts);  // system realtime clock

    // Try to get a free buffer 
    err_code = acquire_meta_lock(p);
    if(err_code) 
        return err_code;  // We haven't acquire a lock here, so just return is ok

    uint8_t qh = p->qhead, qt = p->qtail;
    if(is_later_than(p->timestamp, ts)) { // Just do nothing if not the lastest 
        err_code = 1;
        goto END_METALOCK;
    }
    // Find the lastest data in the circle queue
    while(qh != qt && memcmp(&p->b_timestamp[qh], &p->timestamp, \
                sizeof(struct timespec))){
        qh = ((qh + 1) & (CIRCLE_QUEUE_LENGTH - 1)); 
    }

    // Append the selected buffer to the end of the queue 
    tmp = qt;    // [qh, qt), so we have to write to tmp
    qt = ((qt + 1) & (CIRCLE_QUEUE_LENGTH - 1));
    if(qh == qt) {       // the circle queue is full
        qt = tmp;        // roll back
        err_code = -1;   // Warning, the queue is full, so we should not write to it
        goto END_METALOCK;
    }
    p->b_timestamp[tmp] = ts;

END_METALOCK: 
    p->qtail = qt; p->qhead = qh;
    release_meta_lock(p);
    if(err_code) 
        return err_code;

    // mem copying 
    // The process which get the meta lock can write to the data block
    memcpy(p->data + tmp * size, data, size);

    // The process which write the data is out of meta lock, 
    // so we should acquire it again to write the timestamp
    err_code = acquire_meta_lock(p);
    p->timestamp = ts;
    if(err_code) /* we modify the timestamp aggressively to prevent deadlock */
        return -2;  
    release_meta_lock(p);

    return 0;
}
