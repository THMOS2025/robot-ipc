#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "super_variable.h"

struct data_pack {
    struct timespec ts;
    uint8_t data[1024 * 1024 * 10];
} data;

int main(int argc, char **argv)
{
    super_variable x;

    x = link_super_variable("latency_test", sizeof(struct data_pack));

    for(int i = 0; i < 100000000; ++i) {
        clock_gettime(CLOCK_REALTIME, &data.ts);
        write_super_variable(x, &data, sizeof(struct data_pack));
    }

    return 0;
}

