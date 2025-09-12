/* host_variable.h
 */

#ifndef _H_SUPER_VARIABLE
#define _H_SUPER_VARIABLE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _s_host_variable* host_variable;

// create or link to an existing host variable
host_variable link_host_variable(const char *name, const size_t size);   
int read_host_variable(host_variable p, void *buf, \
        const size_t size, const size_t op_size);
int write_host_variable(host_variable p, const void *data, \
        const size_t size, const size_t op_size);
void unlink_host_variable(host_variable p, const char *name, const size_t size);

#ifdef __cplusplus
}
#endif

#endif
