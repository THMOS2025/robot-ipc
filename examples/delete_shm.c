#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>      // For NAME_MAX
#include <sys/mman.h>    // For shm_unlink

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

#define SHM_DIR "/dev/shm"

int main() {
    DIR *dp;
    struct dirent *entry;

    // Open the shm directory
    dp = opendir(SHM_DIR);
    if (dp == NULL) {
        perror("opendir /dev/shm failed");
        return 1;
    }

    while ((entry = readdir(dp))) {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Construct the shared memory name for shm_unlink
        // shm_unlink expects the name with a leading slash
        char shm_name[NAME_MAX + 2];
        snprintf(shm_name, sizeof(shm_name), "/%s", entry->d_name);

        // Attempt to unlink the shared memory object
        if (shm_unlink(shm_name) == 0) {
            printf("Deleted shm object: %s\n", shm_name);
        } else {
            perror("shm_unlink failed");
        }
    }

    closedir(dp);
    return 0;
}