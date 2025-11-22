#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "host_variable.h"

#define PAYLOAD_SIZE 1024 * 1024 * 10

struct data_pack {
    struct timespec ts;
    uint8_t data[PAYLOAD_SIZE];
} data;

int main(int argc, char **argv)
{
    host_variable x;

    x = link_host_variable("latency_test", sizeof(struct data_pack));

    struct timespec ts, ns, ts1, ns1;
    int64_t diff, diff1;

    char *ptr = malloc(PAYLOAD_SIZE);

    while(1) {
        read_host_variable(x, &data, sizeof(struct data_pack), sizeof(struct data_pack));
        clock_gettime(CLOCK_REALTIME, &ts);
        diff = ts.tv_nsec - data.ts.tv_nsec;
        if(diff < 0) diff += 1000000000ull; 

        clock_gettime(CLOCK_REALTIME, &ts1);
        memcpy(ptr, &data, sizeof(struct data_pack));  // write once
        clock_gettime(CLOCK_REALTIME, &ns1);
        diff1 = ns1.tv_nsec - ts1.tv_nsec;
        printf("ipc delay = %lfms, real delay = %lfms\n", \
                (double)diff / 1e6, (double)diff1 / 1e6);
    }

    unlink_host_variable(x, "latency_test", sizeof(struct data_pack));
    free(ptr);

    return 0;
}

