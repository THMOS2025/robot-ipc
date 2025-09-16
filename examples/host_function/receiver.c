#include <stdio.h>
#include <unistd.h>

#include "host_function_receiver.h"

void* foo(const void *args)
{
    int data = *(int*)args; /* interpret args as int */
    printf("foo called: data = %d ", data);
    static int ret;
    ret = data - 1;
    printf("return = %d\n", ret);
    return (void*)&ret;
    /* Or, disable returning by: return NULL; */
}

int main(int argc, char **argv)
{
    int ret;
    printf("host function receiver\n");
    
    /* first create a dispatcher */
    host_function_dispatcher x;
    x = create_host_function_dispatcher(64);
    if(!x) {
        printf("Can not create host_function_dispatcher\n");
        return -1;
    }

    printf("host_function dispatcher created\n");

    /* then attach a foo to this dispatcher */
    ret = attach_host_function(x, "host_function", foo, sizeof(int), sizeof(int));
    if(ret) {
        printf("Can not attach function: %d\n", ret);
        return ret;
    }
    printf("attached foo to host_function dispatcher\n");

    /* run the dispatcher daemon */
    ret = start_host_function_dispatcher(x);
    if(ret) {
        printf("Can not start dispatcher: %d\n", ret);
        return ret;
    }
    printf("host_function dispatcher started\n");

    /* Stuck the main thread */
    sleep(1);

    /* destroy the dispatcher */
    ret = delete_host_function_dispatcher(x);
    if(ret) {
        printf("Can not delete host_function_dispatcher: %d\n", ret);
        return ret;
    }

    return 0;
}
