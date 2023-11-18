#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <ctype.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_socket, client_sockets[MAX_CLIENTS];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Server socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind server socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Server socket bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Server socket listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    fd_set read_fds;
    int max_fd, activity;

    // Initialize client sockets array
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client_sockets[i] = 0;
    }

    printf("Server is listening on port %s...\n", argv[1]);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        max_fd = server_socket;

        for (int i = 0; i < MAX_CLIENTS; ++i) {
            int client_fd = client_sockets[i];

            if (client_fd > 0) {
                FD_SET(client_fd, &read_fds);
            }

            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
        }

        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity == -1) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        // New connection request
        if (FD_ISSET(server_socket, &read_fds)) {
            int new_socket;

            if ((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len)) == -1) {
                perror("Accept error");
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd is %d, IP is : %s, port : %d\n",
                    new_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // Add new socket to the array of sockets
            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    break;
                }
            }
        }

        // Handle data from clients
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            int client_fd = client_sockets[i];

            if (FD_ISSET(client_fd, &read_fds)) {
                char buffer[BUFFER_SIZE];
                ssize_t recv_size, send_size;

                if ((recv_size = recv(client_fd, buffer, sizeof(buffer), 0)) <= 0) {
                    // Connection closed or error
                    getpeername(client_fd, (struct sockaddr*)&client_addr, &addr_len);
                    printf("Host disconnected, IP %s, port %d\n",
                            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    close(client_fd);
                    client_sockets[i] = 0;
                } else {
                    // Convert the received string to uppercase
                    for (int j = 0; j < recv_size; ++j) {
                        buffer[j] = toupper(buffer[j]);
                    }

                    // Send the converted string back to the client
                    send_size = send(client_fd, buffer, recv_size, 0);
                    if (send_size == -1) {
                        perror("Send error");
                    }
                }
            }
        }
    }

    // Close the server socket
    close(server_socket);

    return 0;
}
