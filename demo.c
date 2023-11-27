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
int listen_socket, max_fd;
while (1) {
    // 1. Initialize fdset
    FD_ZERO(&read_fds);
    FD_SET(listen_socket, &read_fds);

    // 2. Call select()
    select(max_fd + 1, &read_fds, NULL, NULL, NULL);

    // 3. Detect event
    if (FD_ISSET(listen_socket, &read_fds))
    {
        // 4. Handle event
        handle_event();

        // 5. Loop back, continue.
        continue;
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

