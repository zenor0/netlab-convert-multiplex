#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <msg.c>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    setbuf(stdout,NULL);
    const char *server_ip = argv[1];
    const int server_port = atoi(argv[2]);

    // Create socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Display local and remote address information
    struct sockaddr_in local_address, remote_address;
    socklen_t addr_len = sizeof(local_address);
    getsockname(client_socket, (struct sockaddr *)&local_address, &addr_len);
    printf("Local address: %s:%d\n", inet_ntoa(local_address.sin_addr), ntohs(local_address.sin_port));

    addr_len = sizeof(remote_address);
    getpeername(client_socket, (struct sockaddr *)&remote_address, &addr_len);
    printf("Remote address: %s:%d\n", inet_ntoa(remote_address.sin_addr), ntohs(remote_address.sin_port));

    // Read input from user, send to server, and receive response
    char buffer[MAX_BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, MAX_BUFFER_SIZE);
        printf("Enter a string (or 'exit' to quit): ");
        scanf("%s", buffer);

        // Exit if the user enters 'exit'
        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // Send the string to the server
        if (send(client_socket, buffer, strlen(buffer) + 1, 0) == -1)
            printf("fail.\n");

        // Receive the response from the server
        recv(client_socket, buffer, strlen(buffer), 0);
        printf("Server response: %s\n", buffer);
    }

    // Close the socket
    close(client_socket); 
    printf("Socket closed. exiting...\n");

    return 0;
}
