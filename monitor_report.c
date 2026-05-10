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

    strncpy(timebuf, ctime(&now), sizeof(timebuf) - 1);
    timebuf[strcspn(timebuf, "\n")] = '\0';

    int len = snprintf(line, sizeof(line),"[MONITOR][%s] SIGUSR1 primit: un raport nou a fost adaugat.\n", timebuf);
    write(STDOUT_FILENO, line, len);
}

void write_pid_file()
{
    int fd = 0;
    if((fd = open(".monitor_pid", O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
    {
        perror("EROARE OPEN .monitor_pid!\n");
        exit(-1);
    }

    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%d\n", (int)getpid());
    if(write(fd, buf, len) != len)
    {
        perror("EROARE WRITE .monitor_pid!\n");
        close(fd);
        exit(-1);
    }
    close(fd);
}

void remove_pid_file()
{
    if(unlink(".monitor_pid") < 0 && errno != ENOENT)
        perror("EROARE UNLINK .monitor_pid!\n");
}

int main()
{
    struct sigaction sa_int, sa_usr1;

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

    printf("[MONITOR] Pornit. PID=%d. Astept rapoarte noi...\n", (int)getpid());
    fflush(stdout);

    while(keep_running)
        pause();

    printf("[MONITOR] SIGINT primit. Oprire monitor (PID=%d).\n", (int)getpid());
    fflush(stdout);

    remove_pid_file();

    return 0;
}