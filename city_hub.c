#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

void start_monitor()
{
    int pipefd[2];
    if(pipe(pipefd) < 0)
    {
        perror("EROARE PIPE!\n");
        exit(-1);
    }

    pid_t fid = fork();
    if(fid < 0)
    {
        perror("EROARE FORK fid!\n");
        exit(-1);
    }

    if(fid == 0)
    {
        close(pipefd[0]);

        pid_t monitor = fork();
        if(monitor < 0)
        {
            perror("EROARE FORK monitor!\n");
            exit(-1);
        }

        if(monitor == 0)
        {
            if(dup2(pipefd[1], STDOUT_FILENO) < 0)
            {
                perror("EROARE DUP2!\n");
                exit(-1);
            }
            close(pipefd[1]);

            int fd = open(".monitor_pid", O_RDONLY);
            if(fd >= 0)
            {
                char buf[32];
                memset(buf, 0, sizeof(buf));
                read(fd, buf, sizeof(buf) - 1);
                close(fd);
                pid_t existing = (pid_t)atoi(buf);
                if(existing > 0 && kill(existing, 0) == 0)
                {
                    printf("EROARE: Monitor deja pornit (PID=%d).\n", (int)existing);
                    fflush(stdout);
                    exit(0);
                }
            }

            execlp("./monitor_report", "./monitor_report", NULL);
            perror("EROARE EXECLP monitor_report!\n");
            exit(-1);
        }

        close(pipefd[1]);

        char buf[512];
        int n = 0;
        while((n = read(pipefd[0], buf, sizeof(buf) - 1)) > 0)
        {
            buf[n] = '\0';
            printf("[HUB] %s", buf);
            fflush(stdout);

            if(strstr(buf, "Oprire") != NULL || strstr(buf, "EROARE") != NULL)
            {
                printf("[HUB] Monitorul s-a oprit.\n");
                fflush(stdout);
                break;
            }
        }

        close(pipefd[0]);
        waitpid(monitor, NULL, 0);
        exit(0);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    printf("[HUB] hub_mon pornit in background (PID=%d).\n", (int)fid);
}

int main()
{
    char input[256];

    printf("Comenzi: start_monitor | exit\n");

    while(1)
    {
        printf("city_hub> ");
        fflush(stdout);

        if(fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if(strcmp(input, "exit") == 0)
            break;
        else if(strcmp(input, "start_monitor") == 0)
            start_monitor();
        else
            printf("Comanda necunoscuta: %s\n", input);
    }

    return 0;
}