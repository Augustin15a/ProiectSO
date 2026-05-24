#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

volatile sig_atomic_t keep_running = 1;

void handle_sigint(int sig)
{
    (void)sig;
    keep_running = 0;
}

void handle_sigusr1(int sig)
{
    (void)sig;
    time_t now = time(NULL);
    char timebuf[64];
    char line[256];
    int len = 0;

    strncpy(timebuf, ctime(&now), sizeof(timebuf) - 1);
    timebuf[strcspn(timebuf, "\n")] = '\0';

    len = snprintf(line, sizeof(line), "INFO:[MONITOR][%s] SIGUSR1 primit: un raport nou a fost adaugat.\n", timebuf);
    if(write(STDOUT_FILENO, line, len) < 0)
        perror("EROARE WRITE SIGUSR1!\n");
}

void write_pid_file()
{
    int fd = 0;
    char buf[32];
    int len = 0;

    if((fd = open(".monitor_pid", O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
    {
        perror("EROARE OPEN .monitor_pid!\n");
        exit(-1);
    }

    len = snprintf(buf, sizeof(buf), "%d\n", (int)getpid());
    if(write(fd, buf, len) != len)
    {
        perror("EROARE WRITE .monitor_pid!\n");
        if(close(fd) < 0)
            perror("EROARE CLOSE .monitor_pid!\n");
        exit(-1);
    }
    if(close(fd) < 0)
    {
        perror("EROARE CLOSE .monitor_pid!\n");
        exit(-1);
    }
}

void remove_pid_file()
{
    if(unlink(".monitor_pid") < 0 && errno != ENOENT)
        perror("EROARE UNLINK .monitor_pid!\n");
}

int main()
{
    int fd = 0;
    int len = 0;
    char buf[32];
    char line[128];
    struct sigaction sa_int, sa_usr1;

    if((fd = open(".monitor_pid", O_RDONLY)) >= 0)
    {
        memset(buf, 0, sizeof(buf));
        if(read(fd, buf, sizeof(buf) - 1) < 0)
        {
            perror("EROARE READ .monitor_pid!\n");
            if(close(fd) < 0)
                perror("EROARE CLOSE .monitor_pid!\n");
            exit(-1);
        }
        if(close(fd) < 0)
        {
            perror("EROARE CLOSE .monitor_pid!\n");
            exit(-1);
        }
        pid_t existing = (pid_t)atoi(buf);
        if(existing > 0 && kill(existing, 0) == 0)
        {
            len = snprintf(line, sizeof(line), "ERROR:Monitor deja pornit (PID=%d).\n", (int)existing);
            if(write(STDOUT_FILENO, line, len) < 0)
                perror("EROARE WRITE ERROR!\n");
            exit(0);
        }
    }

    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    if(sigaction(SIGINT, &sa_int, NULL) < 0)
    {
        perror("EROARE SIGACTION SIGINT!\n");
        exit(-1);
    }

    memset(&sa_usr1, 0, sizeof(sa_usr1));
    sa_usr1.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = SA_RESTART;
    if(sigaction(SIGUSR1, &sa_usr1, NULL) < 0)
    {
        perror("EROARE SIGACTION SIGUSR1!\n");
        exit(-1);
    }

    write_pid_file();

    len = snprintf(line, sizeof(line), "INFO:[MONITOR] Pornit. PID=%d. Astept rapoarte noi...\n", (int)getpid());
    if(write(STDOUT_FILENO, line, len) < 0)
    {
        perror("EROARE WRITE INFO!\n");
        exit(-1);
    }

    while(keep_running)
        pause();

    len = snprintf(line, sizeof(line), "INFO:[MONITOR] SIGINT primit. Oprire monitor (PID=%d).\n", (int)getpid());
    if(write(STDOUT_FILENO, line, len) < 0)
        perror("EROARE WRITE OPRIRE!\n");

    remove_pid_file();

    return 0;
}