#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "super_function_caller.h"

int main(int argc, char **argv)
{
    printf("super function caller\n");
    super_function_caller x;
    x = link_super_function("stress_test");

    if(!x) {
        perror("Can not link super function !");
        return -1;
    }

    printf("Linked super function\n");

    struct timespec ts;
    for(int i = 0; i < 10000000; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        call_super_function(x, &ts, sizeof(struct timespec));
        if(i % 100000 == 0)
            printf("i = %d\n", i);
    }

    unlink_super_function(x);
    return 0;
}

