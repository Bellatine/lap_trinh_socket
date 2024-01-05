#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct SEND_PARAM_
{
    int c;
    int port;
    char filename[256];
} SEND_PARAM;

void* SendThread(void* arg)
{
    int c = ((SEND_PARAM*)arg)->c;
    int port = ((SEND_PARAM*)arg)->port;
    char filename[256] = { 0 };
    strcpy(filename, ((SEND_PARAM*)arg)->filename);
    free(arg);
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR*)&saddr, sizeof(saddr));
    listen(s, 10);
    int d = accept(s, (SOCKADDR*)&caddr, &clen);
    FILE* f = fopen(filename, "rb");
    char buffer[1024] = { 0 };
    while (!feof(f))
    {
        int r = fread(buffer, 1, sizeof(buffer), f);
        send(d, buffer, r, 0);
    }
    fclose(f);
    close(d);
    send(c, (char*)"DONE", 4, 0);
}

void* CommandThread(void* arg)
{
    int c = *((int*)arg);
    free(arg);
    char buffer[1024] = { 0 };
    while (0 == 0)
    {
        memset(buffer, 0, sizeof(buffer));
        recv(c, buffer, sizeof(buffer) - 1, 0);
        while (buffer[strlen(buffer) - 1] == '\r' || buffer[strlen(buffer) - 1] == '\n')
        {
            buffer[strlen(buffer) - 1] = 0;
        }
        if (strstr(buffer,"GET ") == buffer)
        {
            char command[256] = { 0 };
            char filename[256] = { 0 };
            int port = 0;
            sscanf(buffer, "%s%s%d", command, filename, &port);
            pthread_t pid;
            SEND_PARAM* arg1 = (SEND_PARAM*)calloc(1, sizeof(SEND_PARAM));
            arg1->port = port;
            arg1->c = c;
            strcpy(arg1->filename, filename);
            pthread_create(&pid, NULL, SendThread, (void*)arg1); 
        }
        else if (strcmp(buffer, "QUIT") == 0)
        {
            close(c);
            break;
        }else 
        {
            char* invalid = "INVALID COMMAND";
            send(c, invalid, strlen(invalid), 0);
        }
    }
    return NULL;
}

int main()
{
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5555);
    saddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR*)&saddr, sizeof(saddr));
    listen(s, 10);
    while (0 == 0)
    {
        int c = accept(s, (SOCKADDR*)&caddr, &clen);
        int* arg = (int*)calloc(1, sizeof(int));
        *arg = c;
        pthread_t pid;
        pthread_create(&pid, NULL, CommandThread, (void*)arg);
    }
}