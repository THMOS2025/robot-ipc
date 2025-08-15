#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#include "super_variable.h"
#include "logger.h"

// Calculate the difference between two timespec structs (now - start).
// The result is stored in the result argument.
void timespec_diff(const struct timespec *start, const struct timespec *now, struct timespec *result) {
    result->tv_sec = now->tv_sec - start->tv_sec;
    result->tv_nsec = now->tv_nsec - start->tv_nsec;
    if (result->tv_nsec < 0) {
        result->tv_sec -= 1;
        result->tv_nsec += 1000000000L;
    }
}

int main() {
    // Link to the super variable with the name "test" and element size of int
    super_variable x = link_super_variable("test", 10485760);   // must be the same size with wirter

    if(x == NULL) {
        printf("Can not create super_variable!\n");
        return -1;
    }
    else {
        printf("Created super_variable successfully!\n");
    }

    struct timespec read_time, now_time, diff_time;
    int tmp;
    for(int i = 0; i < 10000; i++) {
        // Get the current time before reading the super variable
        clock_gettime(CLOCK_REALTIME, &now_time);
        
        // Attempt to read the super variable, read_time will store the time data was written
        int result = read_super_variable(x, &tmp, sizeof(int), &read_time);
        robot_log(result, "");
        
        if(tmp < 999999) {
            // Calculate the time difference between now_time and read_time
            timespec_diff(&read_time, &now_time, &diff_time);
            printf("value: %d, now_time: %ld.%09ld, read_time: %ld.%09ld, diff: %ld.%09ld\n", 
                tmp, 
                now_time.tv_sec, now_time.tv_nsec, 
                read_time.tv_sec, read_time.tv_nsec, 
                diff_time.tv_sec, diff_time.tv_nsec
            );
        }

        // Sleep for 0.1 second (100,000,000 nanoseconds)
        struct timespec ts = {0, 100000000L};
        nanosleep(&ts, NULL);
    }

    // Unlink the super variable after finishing
    unlink_super_variable(x, "test", sizeof(int));
    return 0;
}
