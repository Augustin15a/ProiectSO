#include "remove_district.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void remove_district(char *district)
{
    char path[128];
    snprintf(path, sizeof(path), "%s/active_reports-%s", district);
    
    pid_t pid = fork();
    switch(pid)
    {
        case 0:
            execvp(district);
            unlink(path);
        case -1:
            perror("EROARE FORK!\n");
            exit(-1);
        default:
            waitpid(pid);
    }
}