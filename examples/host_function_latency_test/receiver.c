#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdatomic.h>

#include "host_function_receiver.h"

atomic_uint_least32_t called_cnt = 0;

struct timespec diff(struct timespec start, struct timespec end) {
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}


void *foo(const void *args)
{
    atomic_fetch_add(&called_cnt, 1);

    struct timespec sent_ts, curr_ts, diff_ts;
    sent_ts = *(struct timespec*)args;
    clock_gettime(CLOCK_MONOTONIC, &curr_ts);
    diff_ts = diff(sent_ts, curr_ts);
    if(called_cnt % 10000 == 0)
        printf("latency = (%1ld.%09ld), sent_ts=(%5ld.%09ld), curr_ts=(%5ld.%09ld)\n", \
                diff_ts.tv_sec, diff_ts.tv_nsec, \
                sent_ts.tv_sec, sent_ts.tv_nsec, \
                curr_ts.tv_sec, curr_ts.tv_nsec );
    return NULL;
}


int main(int argc, char **argv)
{
    printf("host function receiver\n");
    
    /* first create a dispatcher */
    host_function_dispatcher x[16];
    for(int i = 0; i < 16; ++i)
        x[i] = create_host_function_dispatcher(1);
    printf("host_function dispatcher created\n");

    /* then attach a foo to this dispatcher */
    for(int i = 0; i < 16; ++i)
        attach_host_function(x[i], "stress_test", foo, sizeof(struct timespec), 0);
    printf("attached foo to host_function dispatcher\n");

    /* run the dispatcher daemon */
    for(int i = 0; i < 16; ++i)
        start_host_function_dispatcher(x[i]);
    printf("host_function dispatcher started\n");

    /* Stuck the main thread */
    while(called_cnt < 10000000) {
        sleep(2);
        if(called_cnt % 100000 == 0)
            printf("total call cnt = %d\n", called_cnt);
    }

    return 0;
}
