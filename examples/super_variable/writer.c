#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "super_variable.h"


int main(int argc, char **argv)
{
    super_variable x;
    x = link_super_variable("test", sizeof(int));
    if(x == NULL) {
        perror("can not link to test: \n");
        return -1;
    }

    int data = 100;
    if(write_super_variable(x, &data, sizeof(int)) == 0)
        printf("Write data = %d\n", data);
    else
        perror("can not write: ");

    // unlink_super_variable(x, "test", sizeof(int));
    return 0;
}
