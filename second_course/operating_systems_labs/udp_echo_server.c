#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define ERROR -1
#define SUCCESS 0
#define NO_FLAGS 0

int process_client_request(int sockfd) {
    struct sockaddr_in client_addr;
    char received_data[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);
    int recvfromReturnValue = recvfrom(sockfd, (char *)received_data, BUFFER_SIZE, NO_FLAGS, 
                                        (struct sockaddr *)&client_addr, &addr_len);
    if (recvfromReturnValue == ERROR) {
        perror("recvfrom failed in process_client_request");
        return ERROR;
    }
    printf("Received from client: %s\n", received_data);

    int sendtoReturnValue = sendto(sockfd, (const char *)received_data, recvfromReturnValue, NO_FLAGS, 
                                   (const struct sockaddr *)&client_addr, addr_len);
    if (sendtoReturnValue == ERROR) {
        perror("sendto failed in process_client_request");
        return ERROR;
    }

    printf("Data sent back to client.\n\n");
    return SUCCESS;
}


int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    if (server_addr.sin_addr.s_addr == INADDR_NONE) {
        perror("Invalid IP address format");
        return ERROR;
    }
    server_addr.sin_port = htons(SERVER_PORT);
    socklen_t server_len = sizeof(server_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == ERROR) {
        perror("socket creation failed in main");
        return ERROR;
    }

    int bindReturnValue = bind(sockfd, (const struct sockaddr *)&server_addr, server_len);
    if (bindReturnValue == ERROR) {
        perror("bind failed in main");
        close(sockfd);
        return ERROR;
    }
    printf("UDP server is listening on port %d...\n", SERVER_PORT);
    while (1){
        int processReturnValue = process_client_request(sockfd);
        if (processReturnValue == ERROR) {
            close(sockfd);
            return ERROR;
        }
    }
    close(sockfd);
    return SUCCESS;
}