
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

#include "super_function_receiver.h"


#define EXIT_FAILED(x) { err_code = x; goto FAILED; }


struct _s_super_function_list_node {
    struct _s_super_function_list_node *nxt;
    int req_fd, res_fd;
    super_function foo;
};

struct _s_super_function_dispatcher {
    int epoll_fd;
    pthread_t thread;
    /* a chain of the functions dispatched by this this dispatcher */
    struct _s_super_function_list_node *head;
};


/* private functions */
void *__super_function_dispatcher(void *arg);


super_function_dispatcher 
create_super_function_dispatcher()
{
    super_function_dispatcher p;
    p = malloc(sizeof(struct _s_super_function_dispatcher));
    p->head = NULL;
    if((p->epoll_fd = epoll_create1(0)) < 0) 
        goto FAILED; /* cannot create a epoll */ 
    if(pthread_create(&p->thread, NULL, __super_function_dispatcher, p) != 0)
        goto FAILED; /* cannot create a thread handle */ 
    return p;
FAILED:
    free(p);
    return NULL;
}


int
delete_super_function(super_function_dispatcher p)
{
    for(struct _s_super_function_list_node *tmp = p->head;
            p->head; tmp = p->head, p->head = p->head->nxt) {
        close(tmp->req_fd);
        close(tmp->res_fd);
        free(tmp);
    }
    close(p->epoll_fd);
    free(p);
    return 0;
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
attach_super_function(const char *name, \
        super_function foo, super_function_dispatcher dispatcher)
{

    int err_code = 0;
    struct _s_super_function_list_node *p;
    p = malloc(sizeof(struct _s_super_function_list_node));
    p->req_fd = p->res_fd = 0;
    p->foo = foo;

    /* make sure the path exist */
    if (mkdir(PIPE_NAME_PREFIX, 0700) != 0 && errno != EEXIST)
        EXIT_FAILED(-1);

    /* open the pipes first */
    static char name_buf[NAME_MAX_LENGTH];
    sprintf(name_buf, PIPE_NAME_PREFIX "%s_req", name);
    if((p->req_fd = _try_to_open_pipe(name_buf)) < 0)
        EXIT_FAILED(-1); /* can not open the request pipe */
    sprintf(name_buf, PIPE_NAME_PREFIX "%s_res", name);
    if((p->res_fd = _try_to_open_pipe(name_buf)) < 0)
        EXIT_FAILED(-1); /* can not open the request pipe */

    /* then add this node on the chain */
    p->nxt = dispatcher->head;
    dispatcher->head = p;

    /* listening to this pipe in the epoll */
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = (void*)p;
    if(epoll_ctl(dispatcher->epoll_fd, EPOLL_CTL_ADD, p->req_fd, &ev) == -1)
        EXIT_FAILED(-1); /* can not attach this fd to epoll */

    return err_code;

FAILED: /* we have to free the pointer p */
    if(p->req_fd > 0) close(p->req_fd);
    if(p->res_fd > 0) close(p->res_fd);
    free(p);
    return err_code;
}


int
detach_super_function(super_function foo, super_function_dispatcher dispatcher)
{
    /* prev always points to the pointer referencing to tmp, that is, 
     * *prev = tmp. Briefly, assume the chain structure as:
     *
     *    +---+   +---+   +---+
     * ->-- A *->-- B -->-- C -->-
     *    +---|   +-*-+   +---+
     *        |     ^
     *        |    *nxt=B
     *        *prev=A->nxt, **prev=B
     */
    struct _s_super_function_list_node **prev = &dispatcher->head;
    for(struct _s_super_function_list_node *tmp = dispatcher->head->nxt; 
            tmp; prev = &tmp->nxt, tmp = tmp->nxt) {
        if(tmp->foo == foo) {
            /* firstly try to detach the fd in the epoll */
            if(epoll_ctl(dispatcher->epoll_fd, \
                        EPOLL_CTL_DEL, tmp->req_fd, NULL) == -1) {
                /* Notice we do not remove this function from the chain;
                 * It is still monitored by this epoll */
                return -1;
            }

            *prev = tmp->nxt;
            close(tmp->req_fd); /* don't forget this */
            close(tmp->res_fd);
            free(tmp);
            return 0;
        }
    }
    /* That function isn't dispatched by this dispatcher */
    return -2;
}


int
start_super_function_dispatcher(super_function_dispatcher p)
{
    return pthread_detach(p->thread);
}


void *
__super_function_dispatcher(void *arg) 
{
    super_function_dispatcher p = (super_function_dispatcher)arg;

    struct epoll_event events[MAX_EPOLL_EVENTS];
    struct _s_super_function_list_node *func_node;
    size_t args_sz, ret_sz;
    ssize_t bytes_read;

    while(true) {
        int nfds = epoll_wait(p->epoll_fd, events, MAX_EPOLL_EVENTS, -1);
        if (nfds == -1) {
            /* an error occur, but we have no time to handle it. */
            continue; 
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLIN) {
                /* Data is available to be read from this FIFO */
                func_node = (struct _s_super_function_list_node*)\
                            events[i].data.ptr;

                /* Read the size of the arguments first */
                size_t args_sz;
                bytes_read = read(func_node->req_fd, &args_sz, sizeof(size_t));
                if(bytes_read != sizeof(size_t)) {
                    /* Notices that a negative return indicates error and 
                     * a zero return indicates remote closing the pipe */
                    continue;
                }

                /* Then read the real buffer */
                void *args_buffer = malloc(args_sz), *ret_buffer = NULL;
                bytes_read = read(func_node->req_fd, args_buffer, args_sz);

                /* Call the funtion blockingly */
                func_node->foo(args_buffer, args_sz, &ret_buffer, &ret_sz);

                /* Write to the response pipe if there is returning value */
                if(ret_sz != 0 && ret_buffer != NULL) {
                    if(write(func_node->res_fd, ret_buffer, ret_sz) != ret_sz)
                        ; /* some error may ocurr, but I don't know what to do */
                    free(ret_buffer);
                }
            }
        }
    }
}

 
#undef EXIT_FAILED
