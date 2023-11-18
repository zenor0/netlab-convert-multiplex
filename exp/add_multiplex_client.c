#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int client_socket;
    struct sockaddr_in server_addr;
    fd_set read_fds;

    // Create client socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Client socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server %s:%s\n", argv[1], argv[2]);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(client_socket, &read_fds);

        // Use select to multiplex between standard input and the server socket
        if (select(client_socket + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        // Check for data from standard input
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            // Send random numbers to the server
            srand((unsigned)time(NULL));
            int num1 = rand() % 100;
            int num2 = rand() % 100;
            printf("Sending numbers %d and %d to server %s:%s\n", num1, num2, argv[1], argv[2]);

            // Send the random numbers to the server
            send(client_socket, &num1, sizeof(num1), 0);
            send(client_socket, &num2, sizeof(num2), 0);
        }

        // Check for data from the server
        if (FD_ISSET(client_socket, &read_fds)) {
            int result;
            ssize_t recv_size = recv(client_socket, &result, sizeof(result), 0);

            if (recv_size <= 0) {
                printf("Server disconnected\n");
                break;
            }

            // Display the converted string received from the server
            printf("Received from server: %d\n", result);
        }
    }

    // Close the client socket
    close(client_socket);

    return 0;
}
