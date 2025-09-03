/* host_variable.h
 */

#ifndef _H_SUPER_VARIABLE
#define _H_SUPER_VARIABLE

#include "constant.h"

typedef struct _s_host_variable* host_variable;

// create or link to an existing host variable
host_variable link_host_variable(const char *name, const size_t size);   
int read_host_variable(host_variable p, void *buf, const size_t size);
int write_host_variable(host_variable p, const void *data, const size_t size);
void unlink_host_variable(host_variable p, const char *name, const size_t size);
// struct timespec get_host_variable_timestamp(host_variable p);

#endif
