#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include "host_variable.h"

#define DATA_SIZE 1024

struct data_pack {
    uint32_t data[DATA_SIZE];
    uint32_t chksum;
};

host_variable x;

void *thread_main(void *args)
{
    printf("thrd %02d started\n", (int)(uint64_t)args);

    struct data_pack data;
    while(true) {
        /* construct a data pack */
        data.chksum = 0;
        for(int i = 0; i < DATA_SIZE; ++i) {
            data.data[i] = rand();
            data.chksum ^= data.data[i];
        }

        if(write_host_variable(x, &data, sizeof(struct data_pack)) < 0)
            printf("write failed !\n");
    }
    return NULL;
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    x = link_host_variable("test", sizeof(struct data_pack));

    pthread_t thrd[16];
    for(int i = 0; i < 16; ++i) {
        pthread_create(thrd + i, NULL, thread_main, (void*)(uint64_t)i);
        pthread_detach(thrd[i]);
    }

    sleep(100);
    unlink_host_variable(x, "test", sizeof(struct data_pack));
    return 0;
}
