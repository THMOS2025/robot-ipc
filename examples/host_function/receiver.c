#include <stdio.h>
#include <unistd.h>

#include "host_function_receiver.h"

void* foo(const void *args)
{
    int data = *(int*)args; /* interpret args as int */
    printf("foo called: data = %d\n", data);
    return NULL; /* disable response */
}

int main(int argc, char **argv)
{
    printf("host function receiver\n");
    
    /* first create a dispatcher */
    host_function_dispatcher x;
    x = create_host_function_dispatcher(64);
    printf("host_function dispatcher created\n");

    /* then attach a foo to this dispatcher */
    attach_host_function(x, "test", foo, sizeof(int), 0);
    printf("attached foo to host_function dispatcher\n");

    /* run the dispatcher daemon */
    start_host_function_dispatcher(x);
    printf("host_function dispatcher started\n");

    /* Stuck the main thread */
    while(1) sleep(10000);

    return 0;
}
