#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "super_function_caller.h"


struct _s_super_function_caller {
    int req_fd, res_fd;
    size_t sz_arg, sz_ret;
};


int static inline
_try_to_open_pipe(const char *name)
{
    /* Try to create first, then open it if already exists */
    if (mkfifo(name, 0600) == -1) {
        if (errno != EEXIST) /* other error rather than EEXIST */
            return ERR_PIPE_REQ;
    }

    /* Then open it, return ID */
    return open(name, O_RDWR);
}


super_function_caller
link_super_function(const char *name, const size_t sz_arg, const size_t sz_ret) 
{
    static char name_buf[256];
    super_function_caller p = malloc(sizeof(struct _s_super_function_caller));
    p->sz_arg = sz_arg;
    p->sz_ret = sz_ret;
    
    /* make sure the path exists */
    if (mkdir(PIPE_NAME_PREFIX, 0700) != 0 && errno != EEXIST)
        goto FAILED; /* can not mkdir */
    
    /* connect to the request pipe */
    sprintf(name_buf, PIPE_NAME_PREFIX "%s_req", name);
    if((p->req_fd = _try_to_open_pipe(name_buf)) < 0)
        goto FAILED; /* can not create request pipe */
    

    /* connect to the response pipe */
    sprintf(name_buf, PIPE_NAME_PREFIX "%s_res", name);
    if((p->res_fd = _try_to_open_pipe(name_buf)) < 0)
        goto FAILED; /* can not create response pipe */

    return p;

FAILED:
    if(p->req_fd > 0) close(p->req_fd);
    if(p->res_fd > 0) close(p->res_fd);
    free(p);
    return NULL;
}


int
unlink_super_function(super_function_caller p)
{
    int ret = 0;

    ret = close(p->req_fd);
    if(ret) 
        return ERR_PIPE_CLOSE;

    ret = close(p->res_fd);
    if(ret) 
        return ERR_PIPE_CLOSE;

    free(p);
    return ROBOT_OK;
}


int
call_super_function(super_function_caller p, void *arg)
{
    if(write(p->req_fd, arg, p->sz_arg) != p->sz_arg)
        return -1;
    return 0;
}


int
get_response_super_function(super_function_caller p, void *ret)
{
    if(read(p->res_fd, ret, p->sz_ret) != p->sz_ret)
        return -1;
    return 0;
}
