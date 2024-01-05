#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;

int main() {
    printf("Start client\n");
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKADDR_IN server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    if (s == -1) {
        perror("Error creating UDP socket");
        exit(EXIT_FAILURE);
    }
    struct timeval timeout;
    timeout.tv_sec = 5; 
    timeout.tv_usec = 0;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        perror("Error setting socket timeout");
        close(s);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    char filename[1024];
    printf("File name: ");
    scanf("%s", filename);

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file for writing");
        close(s);
        exit(EXIT_FAILURE);
    }
    
    sendto(s, filename, strlen(filename), 0, (struct sockaddr*)&server_addr, server_addr_len);

    long file_size;
    long total_bytes_received = 0;
    char buffer[1024];
    int isSIZE = 0;

    while( 0 == 0 ){
        if(total_bytes_received == file_size && isSIZE!=0){
            printf("Complete\n");
            break;
        }else if(total_bytes_received > file_size && isSIZE!=0){
            printf("Have an error when tranfer file. \n");
            break;
        }
        int bytes_received = recvfrom(s, buffer,  sizeof(buffer) - 1, 0, (struct sockaddr*)&server_addr, &server_addr_len);
        if (bytes_received <= 0) {
            continue;
        }

        if(strncmp(buffer, "DATA", 4) == 0){
            printf("DATA %s\n",buffer);
            long data_size;
            memcpy(&data_size, buffer + 4, sizeof(int));
            printf("size %ld ---- ", data_size);
            char data_buffer[1024];
            strcpy(data_buffer, buffer + 8);
            printf("buffer %s", data_buffer);
            fwrite(data_buffer, sizeof(char), strlen(data_buffer), file);
            total_bytes_received += data_size; 
        }else if (strncmp(buffer, "SIZE", 4) == 0) {
            if(isSIZE == 0){
                memcpy(&file_size, buffer + 4, sizeof(int));
                printf("SIZE: %ld\n", file_size);
                isSIZE = -1;
            }else{
                printf("Error: 2 SIZE packet. \n");
                break;
            }
            
        }else{
            printf("FAIL\n");
        }

        
    }

    fclose(file);
    close(s);
}

