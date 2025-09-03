#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "super_function_caller.h"

int main(int argc, char **argv)
{
    printf("super function caller\n");
    super_function_caller x;
    x = link_super_function("stress_test", sizeof(struct timespec), 0);

    if(!x) {
        perror("Can not link super function !");
        return -1;
    }

    printf("Linked super function\n");

    struct timespec ts;
    for(int i = 0; 1; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        call_super_function(x, &ts);
        if(i % 1000 == 0)
            printf("i = %d\n", i);
        usleep(1);
    }

    unlink_super_function(x);
    return 0;
}

