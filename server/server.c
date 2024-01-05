#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_PACKET_SIZE 1016
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;

int main() {
    SOCKADDR_IN server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (SOCKADDR*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding");
        close(s);
        exit(EXIT_FAILURE);
    }
    
    while(0==0){
        char filename[256];
        int bytes_received = recvfrom(s, filename, sizeof(filename), 0, (SOCKADDR*)&client_addr, &client_addr_len);
        if (bytes_received <= 0) {
            perror("Error receiving filename");
            close(s);
            exit(EXIT_FAILURE);
        }

        filename[bytes_received] = '\0';

        FILE* file = fopen(filename, "rb");
        if (file == NULL) {
            perror("Error opening file");
            close(s);
            exit(EXIT_FAILURE);
        }

        fseek(file, 0, SEEK_END);
        int file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char packet[MAX_PACKET_SIZE];
        strcpy(packet, "SIZE");
        memcpy(packet + 4, &file_size, sizeof(int));
        int check = sendto(s, packet, strlen(packet), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
        if(check == -1){
            perror("Error send");
            close(s);
            exit(EXIT_FAILURE);
        }
        printf("Check: %s", packet );

        char* buffer = (char*)malloc(file_size);
        fread(buffer, 1, file_size, file);

        int total_sent = 0;
        printf("Total send: %d\n", file_size);
        while (total_sent < file_size) {
            int remaining = file_size - total_sent;
            printf("remaining: %d ----- ", remaining);
            int packet_size = remaining < (MAX_PACKET_SIZE - 8) ? remaining : (MAX_PACKET_SIZE - 8);
            //send_data_packet(server_socket, , packet_size, client_addr);
            char packet_send[MAX_PACKET_SIZE];
            char* data = buffer + total_sent;
            strcpy(packet_send, "DATA");
            int packet_send_size = packet_size + 8;
            memcpy(packet_send + 4, &packet_size, sizeof(int));
            memcpy(packet_send + 8, data, packet_size);
            int check1 = sendto(s, packet_send, packet_send_size, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
            if(check1<0){
                perror("Error send");
                close(s);
                exit(EXIT_FAILURE);
            }
            printf("Check: %s\n", packet_send );
            total_sent += packet_size;
        }
        

        free(buffer);
        fclose(file);
    }
    close(s);
}
