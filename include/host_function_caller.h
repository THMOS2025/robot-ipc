/* host_variable.h
 */

#ifndef _H_SUPER_FUNCTION_CALLER
#define _H_SUPER_FUCNTION_CALLER

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _s_host_function_caller* host_function_caller;

/* Link and unlink a remote function. They are identified by names. */
host_function_caller link_host_function(const char* name, \
        const size_t sz_arg, const size_t sz_ret);
int unlink_host_function(host_function_caller p);

/* Notice: this function will block until the remote return. 
 * This api return the size of returned data from remote, and -1 if failed. */
int call_host_function(host_function_caller p, const void* arg);
int get_response_host_function(host_function_caller p, void *ret_buf);

#ifdef __cplusplus
}
#endif

#endif 
