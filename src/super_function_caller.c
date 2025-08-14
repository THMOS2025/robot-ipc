#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "super_function_caller.h"
#include "constant.h"

struct _s_super_function_caller {
    int req_id, res_id;
};


int static inline _try_to_open_pipe(const char *name)
{
    /* Try to create first, then open it if already exists */
    if (mkfifo(name, 0600) == -1) {
        if (errno != EEXIST) /* other error rather than EEXIST */
            return ERR_PIPE_REQ;
    }

    /* Then open it, return ID */
    return open(name, O_RDWR);
}


super_function_caller link_super_function(const char *name) 
{
    static char name_buf[256];
    super_function_caller p = malloc(sizeof(struct _s_super_function_caller));
    
    /* connect to the request pipe: /tmp/robot_icp/{name}_req */
    sprintf(name_buf, "/tmp/robot_icp_pipe/%s_req", name);
    p->req_id = _try_to_open_pipe(name_buf);

    /* indicate a failed operation */
    if(p->req_id == ERR_PIPE_REQ) { 
        free(p);
        return NULL;
    }

    /* connect to the response pipe: {name}_res */
    sprintf(name_buf, "%s_res", name);
    p->res_id = _try_to_open_pipe(name_buf);


    if(p->res_id == ERR_PIPE_REQ) {
        close(p->req_id);
        free(p);
        return NULL;
    }

    return p;
}


int unlink_super_function(super_function_caller p)
{
    int ret = 0;

    ret = close(p->req_id);
    if(ret) 
        return ERR_PIPE_CLOSE;

    ret = close(p->res_id);
    if(ret) 
        return ERR_PIPE_CLOSE;

    free(p);
    return ROBOT_OK;
}


ssize_t call_super_function(super_function_caller p,
        void *args, size_t args_sz, \
        void *ret_buf)
{
    write(p->req_id, &args_sz, sizeof(size_t));
    write(p->req_id, args, args_sz);

    static ssize_t ret;

    read(p->res_id, &ret, sizeof(size_t));
    read(p->res_id, ret_buf, ret);
    
    return ret;
}

