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

typedef struct 
{
    int reportID;
    int severityLevel;
    char name[32];
    char issueCateg[32];
    char descriptionText[128];
    Coords coord;
    time_t timestamp;
}Report;

#endif