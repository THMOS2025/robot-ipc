#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "host_variable.h"


/* On most platfrom, it is efficient to access memory aligned in a specific 
 * byte length (4 on x32 systems and 8 on x64. So, the compile aligned your 
 * member of a struct to that byte length automatically. However, python
 * ctypes struct may have different behaviour (in most case, the behaviour 
 * has no difference ). To improve compatibility, it is better to use this
 * attribute to disable auto aligned. DO NOT FORGET TO DO SO IN PYTHON END.
 * If you want to use non-fix size struct, you should add this attribute 
 * to prevent compiler from auto padding, unless you can handle the memory
 * offset correctly. */
struct __attribute__((__packed__)) DataFormat 
{
    int x;
    char y[10];
    // void *p; DO NOT SEND POINTER IF YOU ARE SURE THE MEMORY MAP IS GLOBAL
    char appendix[];  // Non-fix size struct
} *data;


int main(int argc, char **argv)
{
    data = (struct DataFormat*)malloc(sizeof(struct DataFormat) + 32);

    host_variable x;
    x = link_host_variable("host_variable_struct", \
            sizeof(struct DataFormat) + 32);  // '10' is the max length for appendix
    if(x == NULL) {
        perror("can not link to test: \n");
        return -1;
    }

    printf("Reader started. \n");
    strcpy(data->appendix + 16, "fixed");
    printf("at %ld\n", data->appendix + 16 - (char*)data);
    printf("Prepared data = {%d, %s, %s, %s}\n", 
            data->x, data->y, data->appendix, data->appendix + 16);
    while(true) {   
        if(read_host_variable(x, data, 
                    sizeof(struct DataFormat) + 32,  // the total size 
                    sizeof(struct DataFormat) + 16    /* the operate size */ ) == 0)        
            printf("Read data = {%d, %s, %s, %s}\n", 
                    data->x, data->y, data->appendix, data->appendix + 16);
        else
            printf("Can not read data\n");
        sleep(1);
    }

    unlink_host_variable(x, "host_variable_struct", \
            sizeof(struct DataFormat) + 32);
    return 0;
}
