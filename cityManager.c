#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

typedef enum level
{
    minor,
    moderate,
    critical,
}level;
typedef struct
{
    float x;
    float y;
}Coords;
typedef struct 
{
    int reportID;
    level severityLevel;
    char inspectorName[30];
    char issueCateg[30];
    char descriptionText[100];
    Coords coord;
    time_t timestamp;
}Report;

void add()
{
    Report rep;
    printf("X: ");
    scanf("%f",&rep.coord.x);
    printf("Y: ");
    scanf("%f",&rep.coord.y);
    printf("Category (road/lighting/flooding/other): ");
    scanf("%s",rep.issueCateg);
    printf("Severity level (1/2/3): ");
    scanf("%d",&rep.severityLevel);
    printf("Description: ");
    scanf("%s",rep.descriptionText);
}

int main(int argc,char **argv)
{
    Report rep;
    add();
    return 0;
}