#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8000
#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 100
#define MAX_ITEMS 1000
typedef struct client_inf
{
    struct sockaddr_in* client;
    char* user;
    char* pass;
};
typedef struct items
{
    int id;
    char* name;
    char* description;
    int price;
    int target_price;
    time_t time;
    int status;
};
typedef struct room
{
    int id;
    struct client_inf* admin;
    struct client_inf client[MAX_CLIENTS];
    struct items list_items[MAX_ITEMS];
};
typedef struct items
{
    int id;
    char* name;
    char* description;
    int price;
    int target_price;
    time_t time;
    int status;
};



int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[MAX_BUFFER_SIZE];
    int num_clients = 0;
    int i;
    struct client_inf client_sockets[MAX_CLIENTS];

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("Error creating server socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding server socket");
        exit(1);
    }

    while (1) {
        int recv_len = recvfrom(server_socket, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len == -1) {
            perror("Error receiving data");
            continue;
        }

        for (i = 0; i < num_clients; i++) {
            if (client_sockets[i].sin_addr.s_addr != client_addr.sin_addr.s_addr) 
            {
                sendto(server_socket, buffer, recv_len, 0, (struct sockaddr*)&client_sockets[i], sizeof(client_sockets[i]));
            }
        }

        int new_client = 1;
        for (i = 0; i < num_clients; i++) {
            if (client_sockets[i].sin_addr.s_addr == client_addr.sin_addr.s_addr) {
                new_client = 0;
                break;
            }
        }
        if (new_client) {
            client_sockets[num_clients] = client_addr;
            num_clients++;
        }
    }

    close(server_socket);
    return 0;
}

