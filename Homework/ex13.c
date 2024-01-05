#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    int x = 5;
    if (fork() == 0)
    {
        printf("Child: %X: %d\n", (unsigned int)&x, x);
        x = x + 1;
        while (0 == 0)
        {
            printf("TEST 1\n");
            sleep(1);
        }
    }else
    {
        if (fork() == 0)
        {
            printf("Child: %X: %d\n", (unsigned int)&x, x);
            x = x + 1;
            while (0 == 0)
            {
                printf("TEST 2\n");
                sleep(2);
            }
        }else        
        {
            if (fork() == 0)
            {
                printf("Child: %X: %d\n", (unsigned int)&x, x);
                x = x + 1;
                while (0 == 0)
                {
                    printf("TEST 3\n");
                    sleep(3);
                }
            }else
            {
                printf("Parent: %X: %d\n", (unsigned int)&x, x);
            }
        }
    }
    getchar();
}