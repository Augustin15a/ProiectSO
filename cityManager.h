#ifndef CITYMANAGER_H
#define CITYMANAGER_H
#include <time.h>

typedef struct
{
    float x;
    float y;
}Coords;

typedef struct 
{
    int reportID;
    int severityLevel;
    char inspectorName[32];
    char issueCateg[32];
    char descriptionText[128];
    Coords coord;
    time_t timestamp;
}Report;

#endif