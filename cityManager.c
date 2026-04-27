#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "cityManager.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include "filter.h"

void printRep(Report rep,int foldergol)
{
    printf("Report numar %d:\n",foldergol);
    printf("User: %s\n", rep.name);
    printf("Coordonate: (%.2f, %.2f)\n", rep.coord.x, rep.coord.y);
    printf("Categorie: %s\n", rep.issueCateg);
    printf("Severitate: %d\n", rep.severityLevel);
    printf("Descriere: %s\n", rep.descriptionText);
    printf("Timestamp: %s\n", ctime(&rep.timestamp));
}
void log_action(char *district, char *role, char *user, char *action)
{
    char path[128];
    char timebuf[64];
    char line[256];
    int fd = 0;
    time_t now = time(NULL);

    snprintf(path, sizeof(path), "%s/logged_district", district);

    if((fd = open(path, O_WRONLY | O_CREAT | O_APPEND, PERM_LOGGED_DISTRICT)) < 0)
    {
        perror("EROARE OPEN logged_district!\n");
        return;
    }
    chmod(path, PERM_LOGGED_DISTRICT);
    struct stat st;
    if(stat(path, &st) == 0)
    {
        if(!(st.st_mode & S_IWUSR))
        {
            perror("EROARE: logged_district nu permite scriere!\n");
            close(fd);
            return;
        }
    }
    strncpy(timebuf, ctime(&now), sizeof(timebuf) - 1);
    timebuf[strcspn(timebuf, "\n")] = '\0';

    snprintf(line, sizeof(line), "[%s] role=%s user=%s action=%s\n", timebuf, role, user, action);
    write(fd, line, strlen(line));
    close(fd);
}
void check_permission(char *path, char *role)
{
    struct stat st;
    if(stat(path, &st) < 0)
    {
        perror("EROARE STAT!\n");
        exit(-1);
    }
    if(strcmp(role, "manager") == 0)
    {
        if(!(st.st_mode & S_IWUSR))
        {
            perror("EROARE: MANAGERUL NU ARE PERMISIUNE DE SCRIERE!\n");
            exit(-1);
        }
    }
    else if(strcmp(role, "inspector") == 0)
    {
        if(!(st.st_mode & S_IWGRP))
        {
            perror("EROARE: INSPECTORUL NU ARE PERMISIUNE DE SCRIERE!\n");
            exit(-1);
        }
    }
}
void add(char *district,char *role,char *user)
{
    Report rep;
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
            perror("EROARE: DOAR MANAGERUL POATE CREA DIRECTOARE!\n");
            exit(-1);
        }
    }

    char path[128];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    int fd = 0;
    if((fd = open(path, O_WRONLY | O_CREAT | O_APPEND, PERM_REPORTS_DAT)) < 0)
    {
        perror("EROARE OPEN!\n");
        exit(-1);
    }
    chmod(path, PERM_REPORTS_DAT);
    check_permission(path, role);

    memset(&rep, 0, sizeof(Report));
    printf("X: "); scanf("%f",&rep.coord.x);
    printf("Y: "); scanf("%f",&rep.coord.y);getchar();
    printf("Category (road/lighting/flooding/other): "); scanf("%s",rep.issueCateg);
    printf("Severity level (1/2/3): "); scanf("%d",&rep.severityLevel);getchar();
    if(!(rep.severityLevel == 1 || rep.severityLevel == 2 || rep.severityLevel == 3))
    {
        perror("EROARE: SECURITY LEVEL NU ESTE IN INTERVALUL [1,3]!\n");
        exit(-1);
    }
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

    char linkname[128];
    char target[128];
    snprintf(linkname, sizeof(linkname), "active_reports-%s", district);
    snprintf(target, sizeof(target), "%s/reports.dat", district);

    struct stat lst;
    if(lstat(linkname, &lst) == 0)
        unlink(linkname);
    symlink(target, linkname);

    log_action(district, role, user, "add");
}
void permisii(mode_t mode, char out[10])
{
    out[0] = (mode & S_IRUSR) ? 'r' : '-';
    out[1] = (mode & S_IWUSR) ? 'w' : '-';
    out[2] = (mode & S_IXUSR) ? 'x' : '-';
    out[3] = (mode & S_IRGRP) ? 'r' : '-';
    out[4] = (mode & S_IWGRP) ? 'w' : '-';
    out[5] = (mode & S_IXGRP) ? 'x' : '-';
    out[6] = (mode & S_IROTH) ? 'r' : '-';
    out[7] = (mode & S_IWOTH) ? 'w' : '-';
    out[8] = (mode & S_IXOTH) ? 'x' : '-';
    out[9] = '\0';
}
void list(char *district,char *role, char *user)
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
        printRep(rep,foldergol);
    }
    if(foldergol == 0)
        printf("IN FOLDER NU EXISTA UN REPORT INCARCAT\n");

    struct stat st;
    if (stat(path, &st) < 0)
    {
        perror("EROARE STAT reports.dat");
        exit(-1);
    }
    char permisiuni[10] = "", timebuf[64] = "";
    strncpy(timebuf, ctime(&st.st_mtime), sizeof(timebuf) - 1);
    timebuf[strcspn(timebuf, "\n")] = '\0';
    permisii(st.st_mode,permisiuni);
    printf("Permisiuni: %s \n Dimensiune: %ld bytes \n Ultima modificare: %s\n\n",permisiuni,(long)st.st_size, timebuf);

    char link[128];
    int link_ok = 0;
    snprintf(link, sizeof(link), "active_reports-%s", district);
    struct stat sl, tinta;
    if(lstat(link, &sl) == 0)
    {
        if(S_ISLNK(sl.st_mode))
        {
            if(stat(link, &tinta) == 0)
                link_ok = 1;
        }
    }
    if(link_ok)
        printf("Symlink: %s activ\n", link);
    else
        printf("AVERTISMENT: %s nu exista sau e invalid!\n", link);

    close(fd);
    log_action(district, role, user, "list");
}
void view(char *district, int reportID,char *role, char *user)
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
    struct stat st;
    if(stat(path, &st) < 0)
    {
        perror("EROARE STAT!\n");
        close(fd);
        exit(-1);
    }
    if(st.st_size / sizeof(Report) < reportID)
    {
        close(fd);
        perror("NU EXISTA IDUL REPORTULUI!\n");
        exit(-1);
    }
    if(lseek(fd, (reportID - 1) * sizeof(Report), SEEK_SET) < 0)
    {
        perror("EROARE LSEEK!\n");
        close(fd);
        exit(-1);
    }
    if(read(fd, &rep, sizeof(Report)) != sizeof(Report))
    {
        perror("EROARE READ!\n");
        close(fd);
        exit(-1);
    }
    printRep(rep,reportID);
    close(fd);
    log_action(district, role, user, "view");
}
void remove_report(char *district, char *role, char *user, int reportID)
{
    if(strcmp(role, "manager") != 0)
    {
        perror("DOAR MANAGERUL POATE STERGE RAPOARTE!\n");
        exit(-1);
    }
    char path[128];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    int fd = 0;
    if((fd = open(path, O_RDWR, PERM_REPORTS_DAT)) < 0)
    {
        perror("EROARE OPEN!\n");
        exit(-1);
    }
    check_permission(path, role);

    struct stat statfolder;
    if(stat(path, &statfolder) < 0)
    {
        perror("EROARE STAT!\n");
        close(fd);
        exit(-1);
    }
    int totalRep = (int)(statfolder.st_size / sizeof(Report));
    if(totalRep < reportID)
    {
        close(fd);
        perror("NU EXISTA IDUL REPORTULUI!\n");
        exit(-1);
    }
    Report rep;
    for(int i = reportID;i < totalRep;i++)
    {
        lseek(fd, i * sizeof(Report), SEEK_SET);
        read(fd, &rep, sizeof(Report));

        rep.reportID = i;

        lseek(fd, (i - 1) * sizeof(Report), SEEK_SET);
        write(fd, &rep, sizeof(Report));
    }

    ftruncate(fd, (totalRep - 1) * sizeof(Report));
    close(fd);
    log_action(district, role, user, "remove_report");
}
void update_threshold(char *district, char *role, char *user, int value)
{
    if(strcmp(role, "manager") != 0)
    {
        perror("DOAR MANAGERUL POATE STERGE RAPOARTE!\n");
        exit(-1);
    }
    char path[128];
    snprintf(path, sizeof(path), "%s/district.cfg", district);

    int fd = 0;
    if((fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, PERM_DISTRICT_CFG)) < 0)
    {
        perror("EROARE OPEN!\n");
        exit(-1);
    }
    chmod(path, PERM_DISTRICT_CFG);
    check_permission(path, role);

    struct stat st;
    if(stat(path,&st) < 0)
    {
        perror("EROARE STAT!\n");
        exit(-1);
    }
    if((st.st_mode & 0777) != PERM_DISTRICT_CFG)
    {
        perror("PERMISIUNE INVALIDA!\n");
        exit(-1);
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "%d\n", value);
    if(write(fd, buf, strlen(buf)) < 0)
    {
        perror("EROARE WRITE!\n");
        close(fd);
        exit(-1);
    }

    close(fd);
    log_action(district, role, user, "update_threshold");
}
int main(int argc,char **argv)
{
    char role[10] = "";
    char user[20] = "";
    char command[32] = "";
    char district[32] = "";
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
            if(i+1 < argc)
            {
                strcpy(district,argv[i+1]);
                i += 1;
            }
        }
    }

    if(strlen(role) == 0)
    {
        perror("EROARE --role ESTE OBLIGATORIU!\n");
        exit(-1);
    }
    if(strlen(user) == 0)
    {
        perror("EROARE --user ESTE OBLIGATORIU!\n");
        exit(-1);
    }
    if(strcmp(role,"manager") != 0 && strcmp(role,"inspector") != 0)
    {
        perror("EROARE ROLUL INTRODUS NU EXISTA!\n");
        exit(-1);
    }

    if(strcmp(command,"add") == 0)
        add(district,role,user);
    if(strcmp(command,"list") == 0)
        list(district, role, user);
    if(strcmp(command,"view") == 0)
        view(district, reportID, role, user);
    if(strcmp(command,"remove_report") == 0)
        remove_report(district, role, user, reportID);
    if(strcmp(command,"update_threshold") == 0)
        update_threshold(district, role, user, value);
    if(strcmp(command,"filter") == 0)
    {
        int conditie = 0;
        for(int i = 1; i < argc; i++)
        {
            if(strcmp(argv[i],"--filter") == 0 && i+1 < argc)
            {
                conditie = i + 2;
                break;
            }
        }
        int num_conds = 0;
        for(int i = conditie; i < argc; i++)
        {
            if(argv[i][0] == '-' && argv[i][1] == '-')
                break;
            num_conds++;
        }
        filter(district, &argv[conditie], num_conds);
    }
    return 0;
}