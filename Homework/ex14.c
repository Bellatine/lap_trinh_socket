#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

void signal_handler(int sig)
{
    if (sig == SIGCHLD)
    {
        int statloc = 0;
        while (waitpid(-1, &statloc, WNOHANG) > 0)
        {
            printf("A child has terminated!\n");
        }
    }
}

int main()
{
    signal(SIGCHLD, signal_handler);
    int x = 5;
    if (fork() == 0)
    {
        printf("Child: %d\n", x);
        x = x + 1;
        exit(0);
    }else
    {
        if (fork() == 0)
        {
            printf("Child: %d\n", x);
            x = x + 1;
            exit(0);
        }else        
        {
            if (fork() == 0)
            {
                printf("Child: %d\n", x);
                x = x + 1;
                exit(0);
            }else
            {
                printf("Parent: %d\n", x);
            }
        }
    }

    getchar();
}