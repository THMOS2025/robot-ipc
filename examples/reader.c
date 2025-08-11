#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "super_variable.h"

int main(int argc, char **argv) {
    super_variable x = link_super_variable("test", sizeof(int));
    if(x == NULL) {
        printf("Can not create super_variable!\n");
        return -1;
    }
    
    int last = -1, tmp;
    for(int i = 0; i < 1000; i++) {
        if(read_super_variable(x, &tmp, sizeof(int))) {
            printf("read failed !");
        }
        if(tmp < 999999)
            printf("%d\n", tmp);
    }
    unlink_super_variable(x, "test", sizeof(int));
    
    return 0;
}
