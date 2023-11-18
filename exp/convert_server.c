#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_BUFFER_SIZE 1024

// Function to convert string to uppercase
void convertToUpper(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const int server_port = atoi(argv[1]);
    // Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(server_port);

    // Bind the socket to the specified address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", server_port);
    while (1) {
        // Accept a client connection
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            perror("Accept failed");
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

        char buffer[MAX_BUFFER_SIZE];
        while (1)
        {
            // memset(buffer, MAX_BUFFER_SIZE, 0);
            // Receive and process the string from the client
            ssize_t bytes_received = recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
            if (bytes_received <= 0) {
                perror("Receive failed");
                close(client_socket);
                break;
            }

            // Convert the string to uppercase
            printf("%s\n", buffer);
            convertToUpper(buffer);
            // Send the converted string back to the client
            send(client_socket, buffer, strlen(buffer), 0);
        }

        // // Close the client socket
        // close(client_socket);
    }

    // Close the server socket
    close(server_socket);

    return 0;
}
