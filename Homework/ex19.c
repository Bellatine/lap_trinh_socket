#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
#define MAX_CLIENT 1024
int g_client_socket[MAX_CLIENT] = { 0 };

void* client_thread(void* arg)
{
    int c = *((int*)arg);
    free(arg);
    arg = NULL;
    char buffer[1024] = { 0 };
    while (0 == 0)
    {
        memset(buffer, 0, sizeof(buffer));
        int error = recv(c, buffer, sizeof(buffer) - 1, 0); 
        if (error > 0)
        {
            for (int i = 0;i < MAX_CLIENT;i++)
            {
                if (g_client_socket[i] > 0 && g_client_socket[i] != c)
                {
                    send(g_client_socket[i], buffer, strlen(buffer), 0);
                }
            }
        }else
            break;
    }

    for (int i = 0;i < MAX_CLIENT;i++)
    {
        if (g_client_socket[i] == c)
        {
            g_client_socket[i] = 0;
            break;
        }
    }
    return NULL;
}
int main()
{
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);
    saddr.sin_port = htons(8888);
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = 0; //0.0.0.0
    bind(s, (SOCKADDR*)&saddr, sizeof(saddr));
    listen(s, 10);
    while (0 == 0)
    {
        int c = accept(s, (SOCKADDR*)&caddr, &clen);
        int i = 0;
        for (i = 0;i < MAX_CLIENT;i++)
        {
            if (g_client_socket[i] == 0)
            {
                g_client_socket[i] = c;
                break;
            }
        }
        if (i < MAX_CLIENT)
        {
            pthread_t id = 0;
            int* arg = (int*)calloc(1, sizeof(int));
            *arg = c;
            pthread_create(&id, NULL, client_thread, (void*)arg);
        }else
        {
            char* reject = "Room is full now, please try again later!\n";
            send(c, reject, strlen(reject), 0);
        }
    }
}