#include <stdio.h>
#include <unistd.h>

#include "super_function_receiver.h"

void foo(const void *args, const size_t args_size, \
        void **ret, size_t *ret_sz)
{
    int data = *(int*)args; /* interpret args as int */
    printf("foo called: data = %d\n", data);
    *ret = NULL; /* one way to disable response */
    *ret_sz = 0; /* another way to disable response */
}

int main(int argc, char **argv)
{
    printf("super function receiver\n");
    
    /* first create a dispatcher */
    super_function_dispatcher x;
    x = create_super_function_dispatcher();
    printf("super_function dispatcher created\n");

    /* then attach a foo to this dispatcher */
    attach_super_function("test", foo, x);
    printf("attached foo to super_function dispatcher\n");

    /* run the dispatcher daemon */
    start_super_function_dispatcher(x);
    printf("super_function dispatcher started\n");

    /* Stuck the main thread */
    while(1) sleep(10000);

    return 0;
}
