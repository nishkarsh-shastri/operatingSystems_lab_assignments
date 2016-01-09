#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

volatile sig_atomic_t got_usr1;

void sigusr1_handler(int sig)
{
    got_usr1 = 1;
    printf("Changed the values\n");
    sleep(2);
}

int main(void)
{
    struct sigaction sa;

    got_usr1 = 0;

    sa.sa_handler = sigusr1_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    // if (sigaction(SIGINT, &sa, NULL) == -1) {
    //     perror("sigaction");
    //     exit(1);
    // }
    signal(SIGINT,sigusr1_handler);
    while (1) {
        printf("PID %d: working hard...\n", getpid());
        printf("Associated\n");        
        sleep(1);
    }

    printf("Done in by SIGUSR1!\n");

    return 0;
}