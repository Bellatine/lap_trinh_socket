#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
char* vnexpress_ip = "111.65.250.2";
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
int main()
{
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = inet_addr(vnexpress_ip);
    int error = connect(s, (SOCKADDR*)&addr, sizeof(addr));
    if (error == 0)
    {
        char* hello = "HELLO\r\n";
        int sent = send(s, hello, strlen(hello), 0);
        printf("Sent %d bytes to vnexpress.net\n", sent);
        char buffer[1024] = { 0 };
        while (0 == 0)
        {
            int received = recv(s, buffer, sizeof(buffer) - 1, 0);
            printf("Received %d bytes from vnexpress.net\n", received);
            if (received <= 0)
            {
                break;
            }else
                printf("%s\n", buffer);
        }
    }else
    {
        printf("Failed to connect\n");
    }
}