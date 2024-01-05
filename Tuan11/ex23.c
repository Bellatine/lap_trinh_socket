#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#define MAX_CLIENT 1024

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
int g_clients[MAX_CLIENT] = { 0 };
int g_count = 0;
fd_set fdread;
int main()
{
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5000);
    saddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR*)&saddr,sizeof(saddr));
    listen(s, 10);
    while (0 == 0)
    {
        struct timeval to;
        to.tv_sec = 0;
        to.tv_usec = 0;
        FD_ZERO(&fdread);
        FD_SET(s, &fdread);
        int r = select(FD_SETSIZE, &fdread, NULL, NULL, &to);
        if (r > 0)
        {
            int c = accept(s, (SOCKADDR*)&caddr, &clen);
            g_clients[g_count++] = c;
        }
        for (int i = 0;i < g_count;i++)
        {
            FD_ZERO(&fdread);
            FD_SET(g_clients[i], &fdread);
            r = select(FD_SETSIZE, &fdread, NULL, NULL, &to);
            if (r > 0)
            {
                char buffer[1024] = { 0 };
                int r = recv(g_clients[i], buffer, sizeof(buffer) - 1, 0);
                if (r > 0)
                {
                    for (int k = 0;k < g_count;k++)
                    {
                        if (g_clients[k] != g_clients[i])
                        {
                            send(g_clients[k], buffer, strlen(buffer), 0);
                        }
                    }
                }
            }
        }
        usleep(10000);
    }
}