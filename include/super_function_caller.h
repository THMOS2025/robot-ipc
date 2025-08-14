/* super_variable.h
 */

#ifndef _H_SUPER_FUNCTION_CALLER
#define _H_SUPER_FUCNTION_CALLER

typedef struct _s_super_function_caller* super_function_caller;

/* Link and unlink a remote function. They are identified by names. */
super_function_caller link_super_function(const char* name);
int unlink_super_function(super_function_caller p);

/* Notice: this function will block until the remote return. 
 * This api return the size of returned data from remote, and -1 if failed. */
ssize_t call_super_function(super_function_caller p, \
        void* args, size_t args_sz, /* args and sizeof args */ \
        void* ret_buf);

#endif 
