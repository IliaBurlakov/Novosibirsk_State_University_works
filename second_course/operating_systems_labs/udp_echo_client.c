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
#define STRING_TERMINATOR_SIZE 1
#define NO_FLAGS 0

int process_server_response(int sockfd) {
    struct sockaddr_in server_addr;
    char message[BUFFER_SIZE];
    socklen_t addr_len = sizeof(server_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    if (server_addr.sin_addr.s_addr == INADDR_NONE) {
        perror("Invalid IP address format");
        return ERROR;
    }
    server_addr.sin_port = htons(SERVER_PORT);

    printf("Enter message to send to the server:\n");
    char* read_data_pointer = fgets(message, BUFFER_SIZE, stdin);
    if (read_data_pointer == NULL) {
        return ERROR;
    }
    size_t message_len = strlen(message) + STRING_TERMINATOR_SIZE;
    int sendtoReturnValue = sendto(sockfd, (const char *)message, message_len, NO_FLAGS,
                                   (const struct sockaddr *)&server_addr, addr_len);
    if (sendtoReturnValue == ERROR) {
        perror("sendto failed in process_server_response");
        return ERROR;
    }

    int recvfromReturnValue = recvfrom(sockfd, (char *)message, sendtoReturnValue, NO_FLAGS,
                                        (struct sockaddr *)&server_addr, &addr_len);
    if (recvfromReturnValue == ERROR) {
        perror("recvfrom failed in process_server_response");
        return ERROR;
    }

    printf("Response from server: %s\n", message);
    return SUCCESS;
}

int main() {
    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == ERROR) {
        perror("socket creation failed in main");
        return ERROR;
    }

    while (1) {
        int processReturnValue = process_server_response(sockfd);
        if (processReturnValue == ERROR) {
            close(sockfd);
            return ERROR;
        }
    }

    close(sockfd);
    return SUCCESS;
}
