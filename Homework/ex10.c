#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;

int main()
{
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKADDR_IN myaddr, toaddr, fromaddr;
    int fromlen = sizeof(fromaddr);
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(5000);
    myaddr.sin_addr.s_addr = 0;
    toaddr.sin_family = AF_INET;
    toaddr.sin_port = htons(8000);
    toaddr.sin_addr.s_addr = inet_addr("172.22.80.1");
    int error = bind(s, (SOCKADDR*)&myaddr, sizeof(myaddr));
    if (error == 0)
    {
        while (0 == 0)
        {
            char buffer[1024] = { 0 };
            fgets(buffer, sizeof(buffer) - 1, stdin);
            sendto(s, buffer, strlen(buffer), 0, (SOCKADDR*)&toaddr, sizeof(toaddr));
            memset(buffer, 0 ,sizeof(buffer));
            int r = recvfrom(s, buffer, sizeof(buffer) - 1, 0, (SOCKADDR*)&fromaddr, &fromlen);
            if (r > 0)
            {
                printf("%s from %s:%d\n", buffer, inet_ntoa(fromaddr.sin_addr), ntohs(fromaddr.sin_port));
            }else
                break;
        }
    }else
        printf("Failed to bind\n");
    close(s);
};