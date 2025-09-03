#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "host_function_caller.h"

int main(int argc, char **argv)
{
    printf("host function caller\n");
    host_function_caller x;
    x = link_host_function("stress_test", sizeof(struct timespec), 0);

    if(!x) {
        perror("Can not link host function !");
        return -1;
    }

    printf("Linked host function\n");

    struct timespec ts;
    for(int i = 0; i < 1000000; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        call_host_function(x, &ts);
        // usleep(1);
    }

    unlink_host_function(x);
    return 0;
}

