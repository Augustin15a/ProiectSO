#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "cityManager.h"

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