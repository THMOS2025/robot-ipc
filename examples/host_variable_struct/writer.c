#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "host_variable.h"

struct __attribute__((__packed__)) DataFormat 
/* If you want to use non-fix size struct, you should add this attribute 
 * to prevent compiler from auto padding. */
{
    int x;
    char y[10];
    // void *p; DO NOT SEND POINTER IF YOU ARE SURE THE MEMORY MAP IS GLOBAL
    char appendix[];  // Non-fix size struct
} *data;


int main(int argc, char **argv)
{
    data = (struct DataFormat*)malloc(sizeof(struct DataFormat) + 32);
    
    data->x = 100;
    strcpy(data->y, "struct");
    strcpy(data->appendix, "change");

    host_variable x;
    x = link_host_variable("host_variable_struct", \
            sizeof(struct DataFormat) + 32);  // '10' is the max length for appendix
    if(x == NULL) {
        perror("can not link to test: \n");
        return -1;
    }

    if(write_host_variable(x, data, 
                sizeof(struct DataFormat) + 32,  // the total size 
                sizeof(struct DataFormat) + 16    /* the operate size */ ) == 0)        
        printf("Write data = {%d, %s, %s, %s}\n", 
                data->x, data->y, data->appendix, data->appendix + 16);
    else
        perror("can not write: ");

    unlink_host_variable(x, "host_variable_struct", sizeof(struct DataFormat));
    return 0;
}
