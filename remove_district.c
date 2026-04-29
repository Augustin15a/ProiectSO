#include "remove_district.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

void remove_district(char *district)
{
    char path[128];
    snprintf(path, sizeof(path), "active_reports-%s", district);
    pid_t pid = fork();
    switch(pid)
    {
        case 0:
            char* argument_list[] = {"rm", "-rf", district,0};
            if(unlink(path) == -1)
            {
                perror("EROARE unlink");
                exit(-1);
            }
            if(execvp("rm",argument_list) == -1)
            {
                perror("EROARE COMANDA NU S-A EXECUTAT CUM TREBUIE!\n");
                exit(-1);
            }
        case -1:
            perror("EROARE FORK!\n");
            exit(-1);
        default:
            waitpid(pid,NULL,0);
    }
}