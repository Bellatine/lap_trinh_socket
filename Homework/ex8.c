#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
int main()
{
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
            int c = accept(s, (SOCKADDR*)&caddr, &clen);
            printf("Client IP: %s\n", inet_ntoa(caddr.sin_addr));
            char* welcome = "Hello World! First TCP Server\n";
            send(c, welcome, strlen(welcome), 0);
            close(c);
        }
    }else
        printf("Failed to bind\n");
    close(s);
}