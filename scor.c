#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "cityManager.h"

int main(int argc, char **argv)
{
    int fd = 0;
    int found = 0;
    int count = 0;
    char path[128];
    char inspectors[64][32];
    int scores[64];
    Report rep;

    if(argc < 2)
    {
        perror("EROARE: DISTRICT LIPSA!\n");
        exit(-1);
    }

    if(snprintf(path, sizeof(path), "%s/reports.dat", argv[1]) < 0)
    {
        perror("EROARE SNPRINTF!\n");
        exit(-1);
    }

    if((fd = open(path, O_RDONLY)) < 0)
    {
        perror("EROARE OPEN reports.dat!\n");
        exit(-1);
    }

    while(read(fd, &rep, sizeof(Report)) == sizeof(Report))
    {
        found = 0;
        for(int i = 0; i < count; i++)
        {
            if(strcmp(inspectors[i], rep.name) == 0)
            {
                scores[i] += rep.severityLevel;
                found = 1;
                break;
            }
        }
        if(!found && count < 64)
        {
            strncpy(inspectors[count], rep.name, 31);
            inspectors[count][31] = '\0';
            scores[count] = rep.severityLevel;
            count++;
        }
    }

    close(fd);

    printf("District: %s\n", argv[1]);
    if(count == 0)
    {
        printf("  (niciun raport)\n");
    }
    else
    {
        for(int i = 0; i < count; i++)
            printf("  Inspector: %-20s | Scor: %d\n", inspectors[i], scores[i]);
    }

    return 0;
}