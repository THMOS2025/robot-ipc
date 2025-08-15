#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "super_function_caller.h"

int main(int argc, char **argv)
{
    srand(time(0));

    printf("super function caller\n");
    super_function_caller x;
    x = link_super_function("test");

    if(!x) {
        perror("Can not link super function !");
        return -1;
    }

    printf("Linked super function\n");

    int args = rand();
    printf("Call super function with args = %d\n", args);
    call_super_function(x, &args, sizeof(args));
    

    unlink_super_function(x);
    return 0;
}

