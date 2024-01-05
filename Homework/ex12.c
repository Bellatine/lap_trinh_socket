#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define INCOMING_PORT 8000
#define OUTGOING_PORT 5000
#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr, broadcast_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[MAX_BUFFER_SIZE];
    int i;

    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    broadcast_addr.sin_port = htons(OUTGOING_PORT);

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("Error creating server socket");
        exit(1);
    }
    int on = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(INCOMING_PORT);

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
        sendto(server_socket, buffer, strlen(buffer), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
    }

    close(server_socket);
    return 0;
}

