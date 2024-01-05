#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
int K = 50000;
int N = 1000;
int S = 0;

void* AddingThread(void* arg )
{
    int time = rand() % 1000;
    usleep(time * 1000);
    int i = *(int*)arg;
    free(arg);
    int startVal = i * (K / N) + 1;
    int endVal = (i + 1) * (K / N);
    for (int i = startVal;i <= endVal;i++)
    {
        S += i;
    }
    return NULL;
}
int main()
{
    srand(time(NULL));
    pthread_t* id = (pthread_t*)calloc(N, sizeof(pthread_t));
    /*for (int i = 0;i <= K;i++)
    {
        S = S + i;
    }*/
    for (int i = 0;i < N;i++)
    {
        int* arg = (int*)calloc(1, sizeof(int));
        *arg = i;
        pthread_create(&id[i], NULL, AddingThread, (void*)arg);
    }

    void* status = NULL;
    for (int i = 0;i < N;i++)
    {
        pthread_join(id[i], (void**)&status);
    }

    free(id);
    id = NULL;
    printf("%d\n", S);
}