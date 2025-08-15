/* super_function_receiver.h
 */

#ifndef _H_SUPER_FUNCTION_RECEIVER
#define _H_SUPER_FUNCTION_RECEIVER

#include "constant.h"

typedef struct _s_super_function_dispatcher* super_function_dispatcher;

/* Define the signature of super functions */
typedef void (*super_function)(const void *args, size_t args_sz, \
        void **ret, size_t *ret_sz);

super_function_dispatcher create_super_function_dispatcher();
int delete_super_function_dispatcher(super_function_dispatcher p);
int attach_super_function(const char* name, \
        super_function foo, super_function_dispatcher dispatcher);
int detach_super_function(super_function foo, \
        super_function_dispatcher dispatcher);

int start_super_function_dispatcher(super_function_dispatcher p);


#endif
