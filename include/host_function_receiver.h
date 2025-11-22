/* host_function_receiver.h
 */

#ifndef _H_SUPER_FUNCTION_RECEIVER
#define _H_SUPER_FUNCTION_RECEIVER

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _s_host_function_dispatcher* host_function_dispatcher;

/* Define the signature of host functions. The size in not explicted given
 * here; it is determinated by the extra info attracted to the epoll callback */
typedef void* (*host_function)(const void *args);

host_function_dispatcher create_host_function_dispatcher(const size_t n);
int delete_host_function_dispatcher(host_function_dispatcher p);
int attach_host_function(host_function_dispatcher p, \
        const char* name, host_function foo, \
        const size_t sz_arg, const size_t sz_ret);
int start_host_function_dispatcher(host_function_dispatcher p);

#ifdef __cplusplus
}
#endif

#endif
