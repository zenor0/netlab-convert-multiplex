#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
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

        // Receive random numbers from the client
        int num1, num2;
        recv(client_socket, &num1, sizeof(num1), 0);
        recv(client_socket, &num2, sizeof(num2), 0);

        // Perform the requested operation (addition in this case)
        int result = num1 + num2;

        // Send the result back to the client
        printf("Receiving: %d and %d, return result %d\n", num1, num2, result);
        send(client_socket, &result, sizeof(result), 0);

        // Close the client socket
        close(client_socket);
    }

    // Close the server socket
    close(server_socket);

    return 0;
}
