/*2. Write a new program called monitor_reports that works as follows:
at startup it creates or overwrites a hidden text file called .monitor_pid where it stores its main process ID. The file is situated
at the same level of the directory tree as the district directories.
when it ends, it deletes the above file
the program only ends when it receives SIGINT, and writes a message on the standard output when it does
the program responds to SIGUSR1 signals (which informs it that a new report has been added) by writing a message on the standard output
*/
#include <stdio.h>
#include <stdlib.h>

int main()
{
    char path[] = "ProiectSO/.monitor_pid";
    int fd = 0;
    if((fd = open(path,O_WRONLY | O_CREAT | O_APPEND,PERM_LOGGED_DISTRICT)) < 0)
    {
        perror("EROARE OPEN .monitor_pid!\n");
        return;
    }
    char arguments_list[] = {"nano",".monitor_pid"};
    if(execvp("nano",arguments_list,0) == -1)

        perror("EROARE EXECVP COMANDA NU S-A EXECUTAT CUM TREBUIE!\n");
        exit(-1);
    }
    return 0;
}