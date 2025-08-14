
#include <stdlib.h>
#include <pthread.h>

#include <sys/epoll.h>

#include "super_function_receiver.h"

struct _s_super_function_list_node {
    struct _s_super_function_list_node *nxt;
#if ALLOW_DELETE_FUNCTION_FROM_DISPATCH
    // used as identity in implement of deleting a function from a dispatcher
    char name[NAME_MAX_LENGTH]; 
#endif
    int req_fd, res_fd;
    super_function foo;
};

struct _s_super_function_dispatcher {
    int epoll_fd;
    pthread_t thread;
    struct _s_super_function_list_node *head;
};


super_function_dispatcher 
create_super_function_dispatcher()
{
    super_function_dispatcher p;
    p = malloc(sizeof(struct _s_super_function_dispatcher));
    p->head = NULL;
    if((p->epoll_fd = epoll_create1(0)) < 0) 
        goto FAILED; /* cannot create a epoll */ 
    if(pthread_create(&p->thread, NULL, my_thread_function, NULL) != 0)
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
attach_super_function_dispatcher(const char *name, \
        super_function foo, super_function_dispatcher dispatcher)
{
#define EXIT_FAILED(x) { err_code = x; goto FAILED; }

    int err_code = 0;
    struct _s_super_function_list_node *p;
    p = malloc(sizeof(struct _s_super_function_list_node));
    p->req_id = p->res_id = 0;
    p->foo = foo;

#if ALLOW_DELETE_FUNCTION_FROM_DISPATCHER
    strcpy(p->name, name);
#endif

    /* open the pipes first */
    static char name_buf[NAME_MAX_LENGTH];
    sprintf(name_buf, PIPE_NAME_PREFIX "%s_req", name);
    if((p->req_fd = _try_to_open(name_buf)) < 0)
        EXIT_FAILED(-1); /* can not open the request pipe */
    sprintf(name_buf, PIPE_NAME_PREFIX "%s_res", name);
    if((p->res_fd = _try_to_open(name_buf)) < 0)
        EXIT_FAILED(-1); /* can not open the request pipe */

    /* then add this node on the chain */
    p->nxt = dispatcher->head;
    dispatcher->head = p;

    /* listening to this pipe in the epoll */
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = p->req_fd;
    if(epoll_ctl(dispatcher->epoll_fd, EPOLL_CTL_ADD, p->req_fd, &ev) == -1)
        EXIT_FAILED(-1); /* can not attach this fd to epoll */

    return err_code;

FAILED: /* we have to free the pointer p */
    if(p->req_id > 0) close(p->req_id);
    if(p->res_id > 0) close(p->res_id);
    free(p);
    return err_code;
#undef EXIT_FAILED(x)
}


void *
__super_function_dispatcher(void *arg) 
{
    super_function_dispatcher p = (super_function_dispatcher)arg;

    struct epoll_event events[MAX_EPOLL_EVENTS];
    while(true) {
        int nfds = epoll_wait(p->epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            /* an error occur, but we have no time to handle it. */
            continue; 
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLIN) {
                // Data is available to be read from this FIFO
                size_t args_sz = 
                int bytes_read = read(events[i].data.fd, buffer, sizeof(buffer));

                if (bytes_read > 0) {
                    // Process the data
                    printf("Read %d bytes from FIFO with fd %d: %.*s\n",
                           bytes_read, events[i].data.fd, bytes_read, buffer);
                } else if (bytes_read == 0) {
                    // End of file (writer closed their end)
                    printf("FIFO with fd %d closed by writer.\n", events[i].data.fd);
                } else {
                    perror("read");
                }
            }
        }
    }
}



#if ALLOW_DELETE_FUNCTION_FROM_DISPATCHER
