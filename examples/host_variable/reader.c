#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "host_variable.h"


int main(int argc, char **argv)
{
    host_variable x;
    x = link_host_variable("host_variable", sizeof(int));
    if(x == NULL) {
        perror("can not link to test: \n");
        return -1;
    }

    printf("Reader started. \n");
    while(true) {
        int data = 0;
        if(read_host_variable(x, &data, sizeof(int)) == 0)
            printf("data = %d\n", data);
        else
            printf("Can not read data\n");
        sleep(1);
    }

    unlink_host_variable(x, "test", sizeof(int));
    return 0;
}
