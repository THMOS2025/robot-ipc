#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "host_variable.h"

struct data_pack {
    struct timespec ts;
    uint8_t data[1024 * 1024 * 10];
} data;

int main(int argc, char **argv)
{
    host_variable x;

    x = link_host_variable("latency_test", sizeof(struct data_pack));

    struct timespec ts;
    int64_t diff;

    while(1) {
        read_host_variable(x, &data, sizeof(struct data_pack));
        clock_gettime(CLOCK_REALTIME, &ts);
        diff = ts.tv_nsec - data.ts.tv_nsec;
        if(diff < 0) diff += 1000000000ull; 
        printf("diff = %lfms curr=%ld,%ld\n", \
                (double)diff / 1e6, ts.tv_sec, ts.tv_nsec);
    }

    return 0;
}

