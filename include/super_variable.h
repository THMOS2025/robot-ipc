/* super_variable.h
 */

#ifndef _H_SUPER_VARIABLE
#define _H_SUPER_VARIABLE

#include "constant.h"

typedef struct _s_super_variable* super_variable;

// create or link to an existing super variable
super_variable link_super_variable(const char *name, const size_t size);   
int read_super_variable(super_variable p, void *buf, const size_t size);
int write_super_variable(super_variable p, const void *data, const size_t size);
void unlink_super_variable(super_variable p, const char *name, const size_t size);
// struct timespec get_super_variable_timestamp(super_variable p);

#endif
