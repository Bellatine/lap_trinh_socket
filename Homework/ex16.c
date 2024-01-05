#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_CLIENT 1000

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;

int s = 0;
int c = 0;
int client_sockets[MAX_CLIENT];
int client_count = 0;
pid_t parentid = 0;

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
    if (sig == SIGUSR1)
    {
        printf("A child process has received some new text\n");
        FILE* f = fopen("tmp.dat","rb");
        fseek(f, 0, SEEK_END);
        int size = ftell(f);
        fseek(f, 0, SEEK_SET);
        char* buffer = (char*)calloc(size + 1, 1);
        fread(buffer, 1, size, f);
        fclose(f);
        for (int i = 0;i < client_count;i++)
        {
            Send(client_sockets[i], buffer, strlen(buffer));
        }
        free(buffer);
    }
}

int main()
{   
    pid_t pid ;
    signal(SIGCHLD, signal_handler);
    signal(SIGUSR1, signal_handler);
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
        listen(s, 10);
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
                            FILE* f = fopen("tmp.dat","wb");
                            if (f != NULL)
                            {
                                fwrite(buffer, 1, strlen(buffer), f);
                                fclose(f);
                            }
                            kill(parentid, SIGUSR1);
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