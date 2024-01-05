#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENT 10
int client_sockets[MAX_CLIENT];
int client_count = 0;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
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
void* handle(void* arg){
    SOCKADDR_IN to_addr;
    int c = *(int*)arg;
    free(arg);
    char buffer[256];
    char res[1024] = "Server Namng response: ";
    int bytes_received = recv(c, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        perror("Error receiving");
        close(c);
        exit(EXIT_FAILURE);
    }
    strcat(res, buffer);
    buffer[bytes_received] = '\0';

    int tmpc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    to_addr.sin_family = AF_INET;
    to_addr.sin_port = htons(9999);
    to_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
    if (connect(tmpc, (SOCKADDR*)&to_addr, sizeof(to_addr)) == 0)
    { 
        printf("Connected to localhost\n");
        Send(tmpc, res, strlen(res));
    }else
        printf("Failed to connect to localhost\n");
    close(tmpc);
    close(c);
    return NULL;
}
int main() {
    printf("Start server\n");
    SOCKADDR_IN server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    

    if (bind(s, (SOCKADDR*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding");
        close(s);
        exit(EXIT_FAILURE);
    }

    listen(s, MAX_CLIENT);

    int i=0;

    while(0==0){
        
        int c = accept(s, (SOCKADDR*)&client_addr, &client_addr_len);
        if(c<=0){
            perror("Error receiving");
            close(s);
            exit(EXIT_FAILURE);
        }
        printf("A new client has connected: %d\n", c);
        pthread_t* id = (pthread_t*)malloc(sizeof(pthread_t));
        int* arg = (int*)malloc(sizeof(int));
        *arg = c;
        pthread_create(id, NULL, handle, (void*)arg);
        void* status = NULL;
        pthread_join(*id, (void**)&status);
        free(id);
        id = NULL;
        //client_sockets[client_count++] = c;
        
    }
    close(s);
}
