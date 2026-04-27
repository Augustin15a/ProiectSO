Pentru generarea celor 2 functii am folosit claude 
Am folosit urmatorul prompt pentru parse_condition:
    am nevoie de o functie
    int parse_condition(const char *input, char *field, char *op, char *value)
    care primeste un string de forma field:operator:value si il imparte in cele
    3 parti. operatorii valizi sunt ==, !=, <, <=, >, >=. returneaza 1 la succes
    si 0 daca e invalid. foloseste doar functii standard pt un citymanager so project
    structura pentru report typedef struct {int reportID;int severityLevel;time_t 
    timestamp;Coords coord;char name[32];char issueCateg[32];char descriptionText[256];
    } Report;
Ce a generat:
    -lucreaza pe o copie locala a input-ului cu strncpy
    -foloseste strchr sa gaseasca primul si al doilea ':' ca delimitatori
    -pune '\0' la locul delimitatorului si copiaza fiecare parte cu strncpy
    -valideaza operatorul fata de cele 6 variante acceptate
    -returneaza 0 daca orice componenta e vida sau operatorul e invalid





Pentru match_condition am folosit urmatorul prompt: 
    folosind aceeasi structura, am nevoie de int match_condition(Report *r,
    const char *field, const char *op, const char *value) care returneaza 1
    daca raportul satisface conditia si 0 altfel. campuri suportate: int severity
    , char issueCateg[32], char name[32],time_t timestamp. pentru campuri string 
    operatorii < <= > >= nu au sens, returneaza 0 in acel caz
Ce a generat:
    -pentru severity: converteste value cu atol si compara numeric
    -pentru category si inspector: face strcmp intre campul din struct si value
    -pentru timestamp: converteste value cu atol si compara cu r->timestamp
    -pentru campuri string cu operatori invalizi afiseaza avertisment si returneaza 0
Ce am invatat
    -trebuie sa fiu atent cand imi generaza ca mai apar probleme
