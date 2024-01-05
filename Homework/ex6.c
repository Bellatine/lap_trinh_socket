#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

char url[1024] = { 0 };
int main()
{
    while (0 == 0)
    {
        printf("URL: ");
        fgets(url, sizeof(url) - 1, stdin);
        while ( url[strlen(url) - 1] == '\r' ||
                url[strlen(url) - 1] == '\n')
        {
            url[strlen(url) - 1] = 0;
        }
        struct addrinfo *result = NULL;
        int n = getaddrinfo(url, "http", NULL, &result);
        if (n == 0)
        {
            struct addrinfo* pOrig = result;
            while (result != NULL)
            {
                struct sockaddr* addr = result->ai_addr;
                struct sockaddr_in* inaddr = (struct sockaddr_in*)addr;    
                printf("%s\n", inet_ntoa(inaddr->sin_addr));
                result = result->ai_next;
            }
            freeaddrinfo(pOrig);
        }else
            printf("Failed to resolve\n");
    }
}
