/* super_function_receiver.h
 */

#ifndef _H_SUPER_FUNCTION_RECEIVER
#define _H_SUPER_FUNCTION_RECEIVER

#include "constant.h"

typedef struct _s_super_function_dispatcher* super_function_dispatcher;
typedef void (*super_function)(void *args, size_t args_sz, \
        void *ret, ssize_t ret_sz);

super_function_dispatcher create_super_function_dispatcher();
int delete_super_function_dispatcher(super_function_dispatcher p);
int attach_super_function_dispatcher(const char* name, \
        super_function foo, super_function_dispatcher dispatcher);

#if ALLOW_DELETE_FUNCTION_FROM_DISPATCHER
int detach_super_function_dispatcher(const char* name);
#endif

int start_super_function_dispatcher(super_function_dispatcher p);


#endif
