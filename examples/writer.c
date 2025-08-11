#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "super_variable.h"

// Allocate 10MB data buffer
char data[10485760];
int ret;

// Get current time in milliseconds
double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

int main(int argc, char **argv) {
    super_variable x = NULL;
    double start, end, elapsed;
    char name[64];

    // (Optional) Initialize data buffer with zeros
    memset(data, 0, sizeof(data));

    for(int i = 0; i < 10000; ++i) {
        // Generate a random variable name for each loop
        snprintf(name, sizeof(name), "test_%d_%d", i, rand());

        // Link (create/attach) a new super_variable with this name
        x = link_super_variable(name, 10485760);
        if(x == NULL) {
            printf("Can not create super_variable!\n");
            return -1;
        }

        // Record start time
        start = get_time_ms();
        // Write the data buffer to the super_variable
        ret = write_super_variable(x, data, 10485760);
        // Record end time
        end = get_time_ms();
        // Calculate elapsed time in milliseconds
        elapsed = end - start;

        if(ret) {
            if(ret == -1)
                printf("Warning: super_variable queue full at i=%d!\n", i);
            else if(ret == -2)
                printf("Warning: acquire_meta_lock failed at i=%d!\n", i);
            else
                printf("write_super_variable failed at i=%d!\n", i);
        }

        // Print progress every 1000 loops
        if((i + 1) % 1000 == 0) {
            printf("Loop %d, time used: %.3f ms\n", i, elapsed);
        }

    }
    unlink_super_variable(x, name, 10485760);
    return 0;
}
