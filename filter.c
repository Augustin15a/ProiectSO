#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include "cityManager.h"
#include "filter.h"

int parse_condition(const char *input, char *field, char *op, char *value)
{
    /* Lucram pe o copie ca sa nu modificam input-ul original */
    char buf[256];
    strncpy(buf, input, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
 
    /* Primul token: field (pana la primul ':') */
    char *ptr = buf;
    char *colon1 = strchr(ptr, ':');
    if (colon1 == NULL)
        return 0;
    *colon1 = '\0';
    strncpy(field, ptr, 32);
    field[31] = '\0';
 
    /* Al doilea token: operator.
     * Operatorii pot fi: ==, !=, <=, >=, <, >
     * Cautam al doilea ':' pornind de dupa primul */
    ptr = colon1 + 1;
    char *colon2 = strchr(ptr, ':');
    if (colon2 == NULL)
        return 0;
    *colon2 = '\0';
    strncpy(op, ptr, 4);
    op[3] = '\0';
 
    /* Validam operatorul — acceptam doar cele 6 variante */
    if (strcmp(op, "==") != 0 && strcmp(op, "!=") != 0 &&
        strcmp(op, "<")  != 0 && strcmp(op, "<=") != 0 &&
        strcmp(op, ">")  != 0 && strcmp(op, ">=") != 0)
        return 0;
 
    /* Al treilea token: value (restul string-ului dupa al doilea ':') */
    ptr = colon2 + 1;
    strncpy(value, ptr, 128);
    value[127] = '\0';
 
    /* Toate cele 3 campuri trebuie sa fie non-vide */
    if (field[0] == '\0' || op[0] == '\0' || value[0] == '\0')
        return 0;
 
    return 1;
}

int compare_op(long lhs, const char *op, long rhs)
{
    if (strcmp(op, "==") == 0) return lhs == rhs;
    if (strcmp(op, "!=") == 0) return lhs != rhs;
    if (strcmp(op, "<")  == 0) return lhs <  rhs;
    if (strcmp(op, "<=") == 0) return lhs <= rhs;
    if (strcmp(op, ">")  == 0) return lhs >  rhs;
    if (strcmp(op, ">=") == 0) return lhs >= rhs;
    return 0;
}

int match_condition(Report *r, const char *field, const char *op, const char *value)
{
    /* --- SEVERITY --- */
    if (strcmp(field, "severity") == 0)
    {
        /* Verificam ca operatorul este valid pentru un camp numeric */
        long val = atol(value);
        return compare_op((long)r->severityLevel, op, val);
    }
 
    /* --- CATEGORY --- */
    if (strcmp(field, "category") == 0)
    {
        /* String — are sens doar == si != */
        if (strcmp(op, "==") != 0 && strcmp(op, "!=") != 0)
        {
            fprintf(stderr, "AVERTISMENT: Operatorul '%s' nu e valid pentru camp string.\n", op);
            return 0;
        }
        int eq = (strcmp(r->issueCateg, value) == 0);
        return (strcmp(op, "==") == 0) ? eq : !eq;
    }
 
    /* --- INSPECTOR (numele userului care a adaugat raportul) --- */
    if (strcmp(field, "inspector") == 0)
    {
        if (strcmp(op, "==") != 0 && strcmp(op, "!=") != 0)
        {
            fprintf(stderr, "AVERTISMENT: Operatorul '%s' nu e valid pentru camp string.\n", op);
            return 0;
        }
        int eq = (strcmp(r->name, value) == 0);
        return (strcmp(op, "==") == 0) ? eq : !eq;
    }
 
    /* --- TIMESTAMP --- */
    if (strcmp(field, "timestamp") == 0)
    {
        /* value este un unix timestamp (numar intreg) */
        long val = atol(value);
        return compare_op((long)r->timestamp, op, val);
    }
 
    fprintf(stderr, "AVERTISMENT: Camp necunoscut: '%s'\n", field);
    return 0;
}

void filter(char *district, char **conditions, int num_conditions)
{
    char fields[16][32];
    char ops[16][4];
    char values[16][128];
    char path[128];
    int fd = 0;
    int index = 0;
    int found = 0;
    Report rep;

    if(num_conditions > 16)
    {
        perror("EROARE: MAXIMUM 16 CONDITII SUPORTATE!\n");
        exit(-1);
    }

    for(int i = 0; i < num_conditions; i++)
    {
        if(!parse_condition(conditions[i], fields[i], ops[i], values[i]))
        {
            perror("EROARE: CONDITIE INVALIDA!\n");
            exit(-1);
        }
    }

    snprintf(path, sizeof(path), "%s/reports.dat", district);

    if((fd = open(path, O_RDONLY)) < 0)
    {
        perror("EROARE OPEN!\n");
        exit(-1);
    }

    while(read(fd, &rep, sizeof(Report)) == sizeof(Report))
    {
        index++;
        int all_match = 1;
        for(int i = 0; i < num_conditions; i++)
        {
            if(!match_condition(&rep, fields[i], ops[i], values[i]))
            {
                all_match = 0;
                break;
            }
        }
        if(all_match)
        {
            printRep(rep, index);
            found++;
        }
    }
    close(fd);

    if(found == 0)
        printf("NICIUN RAPORT NU SATISFACE CONDITIILE DATE\n");
    else
        printf("\n%d raport(e) gasite.\n", found);
}