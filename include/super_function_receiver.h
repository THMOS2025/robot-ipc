/* super_function_receiver.h
 */

#ifndef _H_SUPER_FUNCTION_RECEIVER
#define _H_SUPER_FUNCTION_RECEIVER

#include "constant.h"

typedef struct _s_super_function_dispatcher* super_function_dispatcher;

/* Define the signature of super functions. The size in not explicted given
 * here; it is determinated by the extra info attracted to the epoll callback */
typedef void* (*super_function)(const void *args);

super_function_dispatcher create_super_function_dispatcher(const size_t n);
int attach_super_function(super_function_dispatcher p, \
        const char* name, super_function foo, \
        const size_t sz_arg, const size_t sz_ret);
int start_super_function_dispatcher(super_function_dispatcher p);

#endif
