#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "host_variable.h"

#define PAYLOAD_SIZE 32

struct data_pack {
    struct timespec ts;
    uint8_t data[PAYLOAD_SIZE];
} data;

int main(int argc, char **argv)
{
    host_variable x;

    x = link_host_variable("latency_test", sizeof(struct data_pack));

    while(1) {
        clock_gettime(CLOCK_REALTIME, &data.ts);
        write_host_variable(x, &data, sizeof(struct data_pack), sizeof(struct data_pack));
    }

    unlink_host_variable(x, "latency_test", sizeof(struct data_pack));
    return 0;
}

