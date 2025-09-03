#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "host_variable.h"

#define DATA_SIZE 1024

struct data_pack {
    uint32_t data[DATA_SIZE];
    uint32_t chksum;
};


int main(int argc, char **argv)
{
    host_variable x;
    x = link_host_variable("test", sizeof(struct data_pack));
    if(x == NULL) {
        perror("can not link to test: \n");
        return -1;
    }

    printf("Reader started. \n");
    struct data_pack data;
    while(true) {
        if(read_host_variable(x, &data, sizeof(struct data_pack))) {
            perror("read failed !\n");
            return -1;
        }

        for(int i = 0; i < DATA_SIZE; ++i)
            data.chksum ^= data.data[i];
        if(data.chksum) {
            printf("chksum failed: %d\n", data.chksum);
            return -2;
        }
    }

    unlink_host_variable(x, "test", sizeof(struct data_pack));
    return 0;
}
