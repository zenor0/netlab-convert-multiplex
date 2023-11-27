#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <ctype.h>
#include <sys/utsname.h>
#include <time.h>
#include "msg.c"
#include "myproto.h"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define EMPTY_SOCKET_SIGN -1

#define SERVER_PWD "zenor0"


int disconnect_all_clients(int client_sockets[]);
void show_client_list(int client_sockets[]);
void show_manual();

int main(int argc, char *argv[]) {
    setbuf(stdout, 0);
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_socket, client_sockets[MAX_CLIENTS];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        show_msg(ERROR_TYPE, "Server socket creation failed");
        perror("");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind server socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        show_msg(ERROR_TYPE, "Server socket bind failed: ");
        perror("");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Server socket listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // read_fds records readable file descriptors.
    fd_set read_fds;
    int max_fd, activity;

    // Initialize client sockets array
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client_sockets[i] = EMPTY_SOCKET_SIGN;
    }

    getpeername(server_socket, (struct sockaddr*)&server_addr, &addr_len);
    show_msg(INFO_TYPE, "Server is running on %s:%s...\n", inet_ntoa(server_addr.sin_addr), argv[1]);


    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        max_fd = server_socket;

        // Maintain read_fds and max fd.
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            int client_fd = client_sockets[i];

            if (client_fd > 0) {
                FD_SET(client_fd, &read_fds);
            }

            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
        }

        // max_fd cuz file descriptor starts at index 0
        // select() function will block the procedure until there is a readable file descriptor in read_fds
        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity == -1) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        // Handle stdin
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char input_buffer[BUFFER_SIZE];
            fgets(input_buffer, sizeof(input_buffer), stdin);
            input_buffer[strlen(input_buffer) - 1] = 0;

            if (input_buffer[0] != '\\') {
                show_msg(ERROR_TYPE, "command should start with '\\'. type \\help to get help.\n");
                continue;
            }

            if (strcmp(input_buffer, "\\help") == 0) {
                show_manual();
                continue;
            }

            if (strcmp(input_buffer, "\\exit") == 0) {
                show_msg(INFO_TYPE, "Server is shutting down...\n");
                disconnect_all_clients(client_sockets);
                close(server_socket);
                exit(EXIT_SUCCESS);
                continue;
            }

            if (strcmp(input_buffer, "\\clients") == 0) {
                show_msg(DEBUG_TYPE, "showing clients list...\n");
                show_client_list(client_sockets);
                continue;
            }

            if (strcmp(input_buffer, "\\reset") == 0) {
                if (disconnect_all_clients(client_sockets) == 1)
                    show_msg(INFO_TYPE, "All clients have been disconnected!\n");
                else
                    show_msg(INFO_TYPE, "No client is online.\n");
                continue;
            }
            
            show_msg(ERROR_TYPE, "Unknown command. type \\help to get help.\n");
        }

        // New connection request
        if (FD_ISSET(server_socket, &read_fds)) {
            int new_socket;

            if ((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len)) == -1) {
                perror("Accept error");
                exit(EXIT_FAILURE);
            }


            proto_hdr client_request;
            recv(new_socket, &client_request, sizeof(proto_hdr), 0);
            show_msg(DEBUG_TYPE, "Client %s:%d use %s try to login in.\n", 
                    inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_request.msg);

            if (strcmp(client_request.msg, SERVER_PWD) != 0) {
                show_msg(ERROR_TYPE, "Client %s:%d failed to link.\n",
                        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                send(new_socket, create_response("Server refused!"), sizeof(proto_hdr), 0);
                close(new_socket);
                continue;
            } else {
                show_msg(INFO_TYPE, "Client %s:%d linked.\n",
                        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                send(new_socket, create_response("Server accepted!"), sizeof(proto_hdr), 0);
            }


            // Add new socket to the array of sockets
            // (Append new client if client_sockets isn't nil)
            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (client_sockets[i] == EMPTY_SOCKET_SIGN) {
                    client_sockets[i] = new_socket;
                    break;
                }
            }
        }

        // Handle data from clients
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            int client_fd = client_sockets[i];

            if (FD_ISSET(client_fd, &read_fds)) {
                proto_hdr recv_buffer;
                getpeername(client_fd, (struct sockaddr*)&client_addr, &addr_len);

                if (recv(client_fd, &recv_buffer, sizeof(proto_hdr), 0) <= 0) {
                    // Connection closed or error
                    show_msg(INFO_TYPE, "%s:%d Host disconnected\n",
                            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    close(client_fd);
                    client_sockets[i] = EMPTY_SOCKET_SIGN;
                } else {
                    show_msg(DEBUG_TYPE, "[ %dms ] Received message: %s\n"
                            , systime_delta(recv_buffer.timestamp), recv_buffer.msg);
                    // Convert the received string to uppercase
                    int msg_len = strlen(recv_buffer.msg);
                    for (int j = 0; j < msg_len; ++j) {
                        recv_buffer.msg[j] = toupper(recv_buffer.msg[j]);
                    }
                    recv_buffer.msg[msg_len] = 0;

                    show_msg(DEBUG_TYPE, "Sending response: %s. \tto %s:%d\n"
                            , recv_buffer.msg, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    // Send the converted string back to the client
                    if (send(client_fd, create_response(recv_buffer.msg), sizeof(proto_hdr), 0) == -1) {
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


void show_client_list(int client_sockets[]) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char client_ip[30];
    int client_count = 0;

    // Formatted table header.
    printf("+-------+----------------------+\n");
    printf("| %-5s | %-20s |\n", "fd", "Client IP");
    printf("+-------+----------------------+\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != EMPTY_SOCKET_SIGN)
        {
            client_count++;
            getpeername(client_sockets[i], (struct sockaddr *)&client_addr, &addr_len);
            sprintf(client_ip, "%s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            printf("| %-5d | %-20s |\n", client_sockets[i], client_ip);
        }
    }
    printf("+-------+----------------------+\n");
    show_msg(INFO_TYPE, "%d clients in total.\n", client_count);
}

int disconnect_all_clients(int client_sockets[]) {
    int flag = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != EMPTY_SOCKET_SIGN) {
            close(client_sockets[i]);
            show_msg(DEBUG_TYPE, "Client %d has been disconnected.\n", client_sockets[i]);
            client_sockets[i] = EMPTY_SOCKET_SIGN;
            flag = 1;
        }
    }
    return flag;
}

void disconnect_clients(int client_sockets[], int socket_fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == socket_fd)
        {
            close(socket_fd);
            client_sockets[i] = EMPTY_SOCKET_SIGN;
        }
    }
}

void show_manual() {
    printf("+----------+---------------------------------------+\n");
    printf("| Command  | Description                           |\n");
    printf("|----------|---------------------------------------|\n");
    printf("| \\exit    | Typing to quit program.               |\n");
    printf("| \\clients | Typing to show clients list.          |\n");
    printf("| \\reset   | Typing to disconnect all clients.     |\n");
    printf("| \\help    | Display this help message.            |\n");
    printf("+----------+---------------------------------------+\n");

}

