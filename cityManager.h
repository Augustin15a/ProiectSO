#ifndef CITYMANAGER_H
#define CITYMANAGER_H
#include <time.h>

#define PERM_DISTRICT_DIR     0750   // rwxr-x---
#define PERM_REPORTS_DAT      0664   // rw-rw-r--
#define PERM_DISTRICT_CFG     0640   // rw-r-----
#define PERM_LOGGED_DISTRICT  0644   // rw-r--r--

typedef struct
{
    float x;
    float y;
}Coords;

typedef struct {
    int reportID;
    int severityLevel;
    time_t timestamp;
    Coords coord;
    char name[32];
    char issueCateg[32];
    char descriptionText[256];
} Report;

void printRep(Report rep,int foldergol);
void add(char *district,char *role,char *user);
void permisii(mode_t mode, char out[10]);
void list(char *district, char *role, char *user);
void view(char *district, int reportID, char *role, char *user);
void remove_report(char *district, char *role, char *user, int reportID);
void update_threshold(char *district, char *role, char *user, int value);
void log_action(char *district, char *role, char *user, char *action);
void check_permission(char *path, char *role);
#endif