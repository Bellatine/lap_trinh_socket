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

#define MAX_USER 1024

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef char STRING[1024];
char g_username[1024] = { 0 };

STRING g_listUser[MAX_USER];
STRING g_listIP[MAX_USER];
STRING g_listTime[MAX_USER];
int g_listCount = 0;

void* broadcast_thread(void* arg)
{
    int bsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKADDR_IN baddr;
    baddr.sin_family = AF_INET;
    baddr.sin_port = htons(6000);
    baddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    int on = 1;
    setsockopt(bsocket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
    char msg[2048] = { 0 };
    sprintf(msg, "#DISC %s#", g_username);
    while (0 == 0)
    {
        sendto(bsocket, msg, strlen(msg), 0, (SOCKADDR*)&baddr, sizeof(baddr));
        sleep(1);
    }
    return NULL;
}
void* disc_thread(void* arg)
{
    int bsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKADDR_IN baddr, caddr;
    int clen = sizeof(caddr);
    baddr.sin_family = AF_INET;
    baddr.sin_port = htons(6000);
    baddr.sin_addr.s_addr = 0;
    bind(bsocket, (SOCKADDR*)&baddr, sizeof(baddr));
    char buffer[1024] = { 0 };
    while (0 == 0)
    {
        memset(buffer, 0, sizeof(buffer));
        int error = recvfrom(bsocket, buffer, sizeof(buffer) - 1, 0, (SOCKADDR*)&caddr, (int*)&clen);
        if (error > 0)
        {
            while ( buffer[strlen(buffer) - 1] == '\n' || 
                    buffer[strlen(buffer) - 1] == '\r')
            {
                buffer[strlen(buffer) - 1] = 0;
            }
            if (strstr(buffer,"#DISC")== buffer)
            {
                if (buffer[strlen(buffer) - 1] == '#')
                {
                    buffer[strlen(buffer) - 1] = 0;
                    char command[1024] = { 0 };
                    char username[1024] = { 0 };
                    char unixtime[1024] = { 0 };
                    char ip[1024] = { 0 };
                    sscanf(buffer,"%s%s", command, username);
                    sprintf(unixtime, "%d", (int)time(NULL));
                    strcpy(ip, inet_ntoa(caddr.sin_addr));
                    int i = 0;
                    for (i = 0;i < g_listCount;i++)
                    {
                        if (strcmp(g_listUser[i], username) == 0)
                        {
                            break;
                        }
                    }
                    if (i < g_listCount)
                    {
                        strcpy(g_listTime[i], unixtime);
                    }else
                    {
                        strcpy(g_listUser[g_listCount], username);
                        strcpy(g_listIP[g_listCount], ip);
                        strcpy(g_listTime[g_listCount], unixtime);
                        g_listCount += 1;
                    }
                }
            }
        }
    }
    return NULL;
}
void* tcp_thread(void* arg)
{
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5000);
    saddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR*)&saddr, sizeof(saddr));
    listen(s, 10);
    while (0 == 0)
    {
        int c = accept(s, (SOCKADDR*)&caddr, &clen);
        char chr;
        char filename[1024] = { 0 };
        int count = 0;
        while (count < 2)
        {
            int error = recv(c, &chr, 1, 0);
            if (error > 0)
            {
                if (chr != '#')
                {
                    filename[strlen(filename)] = chr;       
                }else
                    count += 1;
            }else
                break;
        }
        if (count == 2)
        {
            printf("Received file name: %s\n", filename);
            int length = 0;
            int error = recv(c, &length, 4, 0);
            if (error == 4)
            {
                printf("Received file size: %d\n", length);
                int received = 0;
                char buffer[1024] = { 0 };
                while (received < length)
                {
                    error = recv(c, buffer, sizeof(buffer), 0);
                    if (error > 0)
                    {
                        char filepath[2048] = { 0 };
                        sprintf(filepath, "./Download/%s", filename);
                        FILE* f = fopen(filepath,"ab");
                        fwrite(buffer, 1, error, f);
                        fclose(f);
                        received += error;
                    }else
                    {
                        close(c);
                        break;
                    }
                }
            }else
            {
                close(c);
            }
        }else
        {
            close(c);
        }
    }
}
char* unixtime_to_date(char* unixtime)
{
    return unixtime;
}
void print_online_users()
{
    int current = time(NULL);
    for (int i = 0;i < g_listCount;i++)
    {
        int time = atoi(g_listTime[i]);
        if (current - time < 300)
        {
            printf("%s %s %s\n", g_listUser[i], g_listIP[i], unixtime_to_date(g_listTime[i]));
        }
    }
}
void send_file(char* filename, char* username)
{
    printf("Finding: %s\n", username);
    int i = 0;
    for (i = 0;i < g_listCount;i++)
    {
        if (strcmp(g_listUser[i], username) == 0)
        {
            break;
        }
    }
    if (i < g_listCount)
    {
        printf("Preparing: %s\n", g_listIP[i]);
        int c = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        SOCKADDR_IN saddr;
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(5000);
        saddr.sin_addr.s_addr = inet_addr(g_listIP[i]);
        int error = connect(c, (SOCKADDR*)&saddr, sizeof(saddr));
        if (error == 0)
        {
            printf("Connected to: %s\n", g_listIP[i]);
            char buffer[1024] = { 0 };
            sprintf(buffer, "#%s#", filename);
            send(c, buffer, strlen(buffer), 0);
            FILE* f = fopen(filename, "rb");
            fseek(f, 0, SEEK_END);
            int size = ftell(f);
            fseek(f, 0, SEEK_SET);
            send(c, &size, 4, 0);
            while (!feof(f))
            {
                int read = fread(buffer, 1, sizeof(buffer), f);
                send(c, buffer, read, 0);
            }
            fclose(f);
            close(c);
        }
    }
}
int main()
{
    fgets(g_username, sizeof(g_username) - 1, stdin);
    while ( g_username[strlen(g_username) - 1] == '\r' || 
            g_username[strlen(g_username) - 1] == '\n')
    {
        g_username[strlen(g_username) - 1] = 0;
    }

    pthread_t id;
    pthread_create(&id, NULL, broadcast_thread, NULL);
    pthread_create(&id, NULL, disc_thread, NULL);
    pthread_create(&id, NULL, tcp_thread, NULL);
    while (0 == 0)
    {
        char buffer[1024] = { 0 };
        fgets(buffer, sizeof(buffer) - 1, stdin);
        if (strstr(buffer,"LIST") == buffer)
        {
            print_online_users();
        }else if (strstr(buffer,"SEND") == buffer)
        {
            char command[1024] = { 0 };
            char filename[1024] = { 0 };
            char username[1024] = { 0 };
            sscanf(buffer,"%s%s%s", command, filename, username);
            send_file(filename, username);
        }
    }
}