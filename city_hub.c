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
    int n = 0;
    pid_t fid = 0;
    pid_t monitor = 0;
    char buf[512];
    char *line = NULL;
    char *newline = NULL;

    if(pipe(pipefd) < 0)
    {
        perror("EROARE PIPE!\n");
        exit(-1);
    }

    fid = fork();
    if(fid < 0)
    {
        perror("EROARE FORK fid!\n");
        exit(-1);
    }

    if(fid == 0)
    {
        close(pipefd[0]);

        monitor = fork();
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
            if(execlp("./monitor_report", "./monitor_report", NULL) < 0)
            {
                perror("EROARE EXECLP monitor_report!\n");
                exit(-1);
            }
        }

        close(pipefd[1]);

        while((n = read(pipefd[0], buf, sizeof(buf) - 1)) > 0)
        {
            buf[n] = '\0';
            line = buf;
            while((newline = strchr(line, '\n')) != NULL)
            {
                *newline = '\0';

                if(strncmp(line, "ERROR:", 6) == 0)
                {
                    printf("[HUB] EROARE MONITOR: %s\n", line + 6);
                    fflush(stdout);
                    waitpid(monitor, NULL, 0);
                    close(pipefd[0]);
                    exit(0);
                }
                if(strncmp(line, "INFO:", 5) == 0)
                {
                    printf("[HUB] %s\n", line + 5);
                    fflush(stdout);

                    if(strstr(line, "Oprire") != NULL)
                    {
                        printf("[HUB] Monitorul s-a oprit.\n");
                        fflush(stdout);
                        waitpid(monitor, NULL, 0);
                        close(pipefd[0]);
                        exit(0);
                    }
                }

                line = newline + 1;
            }
        }

        close(pipefd[0]);
        waitpid(monitor, NULL, 0);
        exit(0);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    printf("[HUB] hub_mon pornit in background (PID=%d).\n", (int)fid);
    fflush(stdout);
}

void calculate_scores(char **districts, int count)
{
    int pipes[64][2];
    int n = 0;
    pid_t pids[64];
    char buf[512];

    for(int i = 0; i < count; i++)
    {
        if(pipe(pipes[i]) < 0)
        {
            perror("EROARE PIPE scorer!\n");
            exit(-1);
        }

        pids[i] = fork();
        if(pids[i] < 0)
        {
            perror("EROARE FORK scorer!\n");
            exit(-1);
        }

        if(pids[i] == 0)
        {
            if(dup2(pipes[i][1], STDOUT_FILENO) < 0)
            {
                perror("EROARE DUP2 scorer!\n");
                exit(-1);
            }
            close(pipes[i][0]);
            close(pipes[i][1]);
            if(execlp("./scor", "./scor", districts[i], NULL) < 0)
            {
                perror("EROARE EXECLP scorer!\n");
                exit(-1);
            }
        }

        close(pipes[i][1]);
    }

    printf("\n=== Raport workload ===\n");
    fflush(stdout);

    for(int i = 0; i < count; i++)
    {
        while((n = read(pipes[i][0], buf, sizeof(buf) - 1)) > 0)
        {
            buf[n] = '\0';
            printf("%s", buf);
            fflush(stdout);
        }
        close(pipes[i][0]);
        waitpid(pids[i], NULL, 0);
    }

    printf("======================\n");
    fflush(stdout);
}

int main()
{
    char input[512];
    char *token = NULL;
    char *districts[64];
    int count = 0;

    printf("Comenzi: start_monitor | calculate_scores <d1> [d2 ...] | exit\n");

    while(1)
    {
        printf("city_hub> ");
        fflush(stdout);

        if(fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if(strcmp(input, "exit") == 0)
            break;
        if(strcmp(input, "start_monitor") == 0)
            start_monitor();
        else if(strncmp(input, "calculate_scores", 16) == 0)
        {
            count = 0;
            token = strtok(input + 16, " ");
            while(token != NULL && count < 64)
            {
                districts[count++] = token;
                token = strtok(NULL, " ");
            }

            if(count == 0)
                printf("[HUB] Folosire: calculate_scores <district1> [district2 ...]\n");
            else
                calculate_scores(districts, count);
        }
        else
            printf("Comanda necunoscuta: %s\n", input);
    }

    return 0;
}