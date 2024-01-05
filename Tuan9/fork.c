#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define MAX_CLIENT 10
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
int s = 0;
int c = 0;
int client_sockets[MAX_CLIENT];
int client_count = 0;
pid_t parentid = 0;
void signal_handler(int sig)
{
    printf("Exit!\n");
    close(s);
    close(c);
}

int Send(int c, char* data, int len)
{
    int sent = 0;
    while (sent < len)
    {
        int tmp = send(c, data + sent, len - sent, 0);
        if (tmp > 0)
            sent += tmp;
        else
            break;
    }
    if (sent < len)
        return 0;
    else
        return 1;
}

int main()
{
    pid_t pid ;
    signal(SIGINT, signal_handler);
    parentid = getpid();
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.s_addr = 0;//ANY ADDRESS
    int error = bind(s, (SOCKADDR*)&saddr, sizeof(saddr));
    if (error == 0)
    {
        listen(s, MAX_CLIENT);
        while (0 == 0)
        {
            c = accept(s, (SOCKADDR*)&caddr, &clen);
            if (c > 0)
            {
                printf("A new client has connected: %d\n", c);
                client_sockets[client_count++] = c;
                if((pid = fork()) == 0 ){
                    close(s);
                    char buffer[1024] = { 0 };
                    char command[2048] = { 0 };
                    while (0 == 0)
                    {
                        int r = recv(c, buffer, sizeof(buffer) - 1, 0);
                        if (r > 0)
                        {
                            kill(parentid, SIGUSR1);
                            while   (buffer[strlen(buffer) - 1] == '\r' || 
                                    buffer[strlen(buffer) - 1] == '\n')
                            {
                                buffer[strlen(buffer) - 1] = 0;    
                            }
                            sprintf(command, "%s > tmp.txt", buffer);
                            system(command);
                            FILE* f = fopen("tmp.txt","rt");
                            while (!feof(f))
                            {
                                memset(buffer, 0, sizeof(buffer));
                                fgets(buffer, sizeof(buffer) - 1, f);
                                if (Send(c, buffer, strlen(buffer)) == 0)
                                {
                                    break;
                                }
                            }
                            fclose(f);
                        }else
                        {
                            printf("Failed to receive\n");
                            break;
                        }
                    }
                    close(c);
                    exit(0) ;
                }
            }else
            {
                printf("Failed to accept\n");
                break;
            }
        }
    }else
        printf("Failed to bind\n");
}