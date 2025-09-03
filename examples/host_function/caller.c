#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "host_function_caller.h"

int main(int argc, char **argv)
{
    srand(time(0));

    printf("host function caller\n");
    host_function_caller x;
    x = link_host_function("test", sizeof(int), 0);

    if(!x) {
        perror("Can not link host function !");
        return -1;
    }

    printf("Linked host function\n");

    int args = rand();
    printf("Call host function with args = %d\n", args);
    call_host_function(x, &args);
    

    unlink_host_function(x);
    return 0;
}

