#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/types.h>

#include "robot_ipc_constant.h"
#include "host_function_receiver.h"
#include "config.h"


#define _DISPATCHER_EXIT        -1
#define _DISPATCHER_SUSPEND     -2


struct _s_host_function_dispatcher {
    /* variables for running the dispatcher */
    int epoll_fd, to_daemon_fd[2], from_daemon_fd[2];
    pthread_t thread;
    
    /* functions to be dispatcher */
    size_t cnt_func;
    int *req_fd, *res_fd;
    host_function *foo;
    size_t *sz_arg, *sz_ret;
};


/* private functions the entry of pthread  */
static void *__host_function_dispatcher(void *arg);


host_function_dispatcher 
create_host_function_dispatcher(const size_t n)
{
    host_function_dispatcher p;
    p = malloc(sizeof(struct _s_host_function_dispatcher));

    /* initialize epoll */
    if(pipe(p->to_daemon_fd) == -1 || pipe(p->from_daemon_fd) == -1)
        goto FAILED_PIPE; /* cannot create a pipe */
    if((p->epoll_fd = epoll_create1(0)) < 0) 
        goto FAILED_EPOLL; /* cannot create a epoll */ 

    /* attach the control pipe to epoll */
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.u64 = (uint64_t)(-1);
    if (epoll_ctl(p->epoll_fd, EPOLL_CTL_ADD, p->to_daemon_fd[0], &ev) == -1)
        goto FAILED_EPOLL_CTL; 

    /* create the pthread */
    if(pthread_create(&p->thread, NULL, __host_function_dispatcher, p) != 0)
        goto FAILED_PTHREAD; /* cannot create a thread handle */ 

    /* malloc memory for storing infos of functions to be dispatched */
    p->cnt_func = 0;
    p->req_fd = malloc(n * sizeof(int));
    p->res_fd = malloc(n * sizeof(int));
    p->foo = malloc(n * sizeof(host_function));
    p->sz_arg = malloc(n * sizeof(size_t));
    p->sz_ret = malloc(n * sizeof(size_t));
    return p;

FAILED_PTHREAD:
FAILED_EPOLL_CTL:
    close(p->epoll_fd);
FAILED_EPOLL:
FAILED_PIPE:
    free(p);
    return NULL;
}


int
delete_host_function_dispatcher(host_function_dispatcher p)
{
    /* wait for the sub process to exit */
    int ret, tmp = _DISPATCHER_EXIT;
    tmp = write(p->to_daemon_fd[1], &tmp, sizeof(int));
    tmp = read(p->from_daemon_fd[0], &ret, sizeof(int)); 

    /* close fd */
    ret |= close(p->epoll_fd);
    ret |= close(p->to_daemon_fd[0]);
    ret |= close(p->to_daemon_fd[1]);
    ret |= close(p->from_daemon_fd[0]);
    ret |= close(p->from_daemon_fd[1]);
    for(size_t i = 0; i < p->cnt_func; ++i) {
        ret |= close(p->req_fd[i]);
        ret |= close(p->res_fd[i]);
    }
    
    /* free the space */
    free(p->req_fd);
    free(p->res_fd);
    free(p->foo);
    free(p->sz_arg);
    free(p->sz_ret);
    free(p);
    
    return ret;
}


int static inline
_try_to_open_pipe(const char *name)
{
    /* Try to create first, then open it if already exists */
    if (mkfifo(name, 0600) == -1) {
        if (errno != EEXIST) /* other error rather than EEXIST */
            return -1;
    }

    /* Then open it */
    return open(name, O_RDWR);
}
    

int
attach_host_function(host_function_dispatcher p, \
        const char *name, host_function foo, \
        const size_t sz_arg, const size_t sz_ret)
{
    int err_code = 0;
    int id = p->cnt_func++;
    p->sz_arg[id] = sz_arg;
    p->sz_ret[id] = sz_ret;
    p->foo[id] = foo;

    /* make sure the path exist */
    if (mkdir(PIPE_NAME_PREFIX, 0700) != 0 && errno != EEXIST) {
        err_code = -1;
        goto FAILED_MKDIR;
    }

    /* open the pipes first */
    static char name_buf[NAME_MAX_LENGTH];
    sprintf(name_buf, PIPE_NAME_PREFIX "%s_req", name);
    if((p->req_fd[id] = _try_to_open_pipe(name_buf)) < 0) {
        err_code = -2;
        goto FAILED_PIPE_REQ; /* can not open the request pipe */
    }
    sprintf(name_buf, PIPE_NAME_PREFIX "%s_res", name);
    if((p->res_fd[id] = _try_to_open_pipe(name_buf)) < 0) {
        err_code = -2;
        goto FAILED_PIPE_RES; /* can not open the request pipe */
    }

    /* listening to this pipe in the epoll */
    struct epoll_event ev = {
        .events = EPOLLIN, 
        .data.u64 = id,
    };
    if(epoll_ctl(p->epoll_fd, EPOLL_CTL_ADD, p->req_fd[id], &ev) == -1) {
        err_code = -3;
        goto FAILED_EPOLL_CTL; /* can not attach this fd to epoll */
    }

    return err_code;

FAILED_EPOLL_CTL:
    close(p->res_fd[id]);
FAILED_PIPE_RES:
    close(p->req_fd[id]);
FAILED_PIPE_REQ:
FAILED_MKDIR:
    --p->cnt_func;
    return err_code;
}


int
start_host_function_dispatcher(host_function_dispatcher p)
{
    return pthread_detach(p->thread);
}


static void *
__host_function_dispatcher(void *arg) 
{
    host_function_dispatcher p = (host_function_dispatcher)arg;

    struct epoll_event events[MAX_EPOLL_EVENTS];
    size_t args_sz, ret_sz;
    ssize_t bytes_read;
    uint64_t id;
    bool should_exit = false;

    while(!should_exit) {
        int nfds = epoll_wait(p->epoll_fd, events, MAX_EPOLL_EVENTS, -1);
        if (nfds == -1) {
            /* an error occur, but we have no time to handle it. */
            continue; 
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLIN) {
                /* Data is available to be read from this FIFO */
                id = events[i].data.u64;
                
                if(id == (uint64_t)(-1)) { /* control signal */
                    int tmp;
                    bytes_read = read(p->to_daemon_fd[0], &tmp, sizeof(int));
                    switch(tmp) {
                    case _DISPATCHER_EXIT:
                        tmp = 0;
                        should_exit = 1;
                        break;
                    }
                    tmp = write(p->from_daemon_fd[1], &tmp, sizeof(int));
                    continue;
                }


                /* Then read the real buffer */
                void *arg_buffer = malloc(p->sz_arg[id]);
                bytes_read = read(p->req_fd[id], arg_buffer, p->sz_arg[id]);
                if(bytes_read != p->sz_arg[id]) {
                    /* Something wrong happened */
                    continue;
                }

                /* Call the funtion blockingly */
                void *ret_buffer = p->foo[id](arg_buffer);

                /* Write to the response pipe if there is returning value */
                if(ret_buffer != NULL && p->sz_ret[id]) {
                    if(write(p->res_fd[id], ret_buffer, p->sz_ret[id]) != \
                            p->sz_ret[id])
                        ; /* some error may ocurr, but I don't know what to do */
                }
            }
        }
    }

    return NULL;
}

 
#undef _DISPATCHER_EXIT
#undef _DISPATCHER_SUSPEND
