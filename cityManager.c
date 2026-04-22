#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "cityManager.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

void add(char *district,char *role,char *user)
{
    struct stat statdir;
    if (stat(district, &statdir) < 0) 
    {
        if (strcmp(role, "manager") == 0)
        {
            if (mkdir(district, PERM_DISTRICT_DIR) < 0)
            {
                perror("EROARE MKDIR!\n");
                exit(1);
            }
            chmod(district, PERM_DISTRICT_DIR);
        }
        else
        {
            perror("DOAR MANAGERUL POATE CREA DIRECTOARE!\n");
            exit(-1);
        }
    }
    
    char path[128];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    int fd = 0;
    if((fd = open(path, O_WRONLY | O_CREAT | O_APPEND, PERM_REPORTS_DAT)) < 0)//append scriere
    {
        perror("EROARE OPEN!\n");
        exit(-1);
    }
    Report rep;
    memset(&rep, 0, sizeof(Report));
    printf("X: "); scanf("%f",&rep.coord.x);
    printf("Y: "); scanf("%f",&rep.coord.y);getchar();
    printf("Category (road/lighting/flooding/other): "); scanf("%s",rep.issueCateg);
    printf("Severity level (1/2/3): "); scanf("%d",&rep.severityLevel);getchar();
    printf("Description: "); scanf("%255[^\n]",rep.descriptionText);
    strcpy(rep.name,user);
    struct stat statfolder;
    if(stat(path,&statfolder) == 0)
        rep.reportID = (statfolder.st_size / sizeof(Report)) + 1;
    else
        rep.reportID = 1;
    rep.timestamp = time(NULL);

    if(write(fd, &rep, sizeof(Report)) != sizeof(Report))
    {
        perror("EROARE WRITE!\n");
        close(fd);
        exit(-1);
    }
    close(fd);
}
void list(char *district)
{
    char path[128];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    int fd = 0;
    if((fd = open(path, O_RDONLY)) < 0)
    {
        perror("EROARE OPEN!\n");
        exit(-1);
    }
    Report rep;
    int foldergol = 0;
    while(read(fd,&rep,sizeof(Report)) == sizeof(Report))
    {
        foldergol++;
        printf("Report numar %d:\n",foldergol);
        printf("User: %s\n", rep.name);
        printf("Coordonate: (%.2f, %.2f)\n", rep.coord.x, rep.coord.y);
        printf("Categorie: %s\n", rep.issueCateg);
        printf("Severitate: %d\n", rep.severityLevel);
        printf("Descriere: %s\n", rep.descriptionText);
        printf("Timestamp: %s\n", ctime(&rep.timestamp));
    }
    if(foldergol == 0)
        printf("IN FOLDER NU EXISTA UN REPORT INCARCAT\n");
    close(fd);
}
int main(int argc,char **argv)
{
    char role[10];
    char user[20];
    char command[32];
    char district[32];
    char condition[64];
    int reportID;
    int value;
    for(int i = 1; i < argc;i++)
    {
        if(strcmp(argv[i],"--role") == 0)
        {
            strcpy(role,argv[i+1]);
            i++;
        }
        if(strcmp(argv[i],"--user") == 0)
        {
            strcpy(user,argv[i+1]);
            i++;
        }
        if(strcmp(argv[i],"--add") == 0)
        {
            strcpy(command,"add");
            if(i+1 < argc)
                strcpy(district,argv[i+1]);
            i++;
        }
        if(strcmp(argv[i],"--list") == 0)
        {
            strcpy(command,"list");
            if(i+1 < argc)
                strcpy(district,argv[i+1]);
            i++;
        }
        if(strcmp(argv[i],"--view") == 0)
        {
            strcpy(command,"view");
            if(i+2 < argc)
            {
                strcpy(district,argv[i+1]);
                reportID = atoi(argv[i+2]);
                i += 2;
            }
        }
        if(strcmp(argv[i],"--remove_report") == 0)
        {
            strcpy(command,"remove_report");
            if(i+2 < argc)
            {
                strcpy(district,argv[i+1]);
                reportID = atoi(argv[i+2]);
                i += 2;
            }
        }
        if(strcmp(argv[i],"--update_threshold") == 0)
        {
            strcpy(command,"update_threshold");
            if(i+2 < argc)
            {
                strcpy(district,argv[i+1]);
                value = atoi(argv[i+2]);
                i += 2;
            }
        }
        if(strcmp(argv[i],"--filter") == 0)
        {
            strcpy(command,"filter");
            if(i+2 < argc)
            {
                strcpy(district,argv[i+1]);
                strcpy(condition,argv[i+2]);
                i += 2;
            }
        }
    }
    if(strcmp(command,"add") == 0)
        add(district,role,user);
    if(strcmp(command,"list") == 0)
        list(district);
}