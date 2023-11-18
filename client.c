#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include "msg.c"
#include "myproto.h"

#define BUFFER_SIZE 1024


void show_manual();

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port> <server_pwd>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int client_socket;
    struct sockaddr_in server_addr;
    fd_set read_fds;

    // Create client socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        show_msg(ERROR_TYPE, "Client socket creation failed");
        perror("");
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

    // Sending client info to server
    send(client_socket, create_request(argv[3]), sizeof(proto_hdr), 0);

    proto_hdr accept_resp;
    recv(client_socket, &accept_resp, sizeof(accept_resp), 0);


    show_msg(INFO_TYPE, "Connected to the server %s:%s\n", argv[1], argv[2]);

    show_msg(DEBUG_TYPE, "Received server response: \n");
    print_proto_info(accept_resp);

    show_msg(INFO_TYPE, "Server response: %s\n", accept_resp.msg);
    

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
            char input_buffer[BUFFER_SIZE];
            fgets(input_buffer, sizeof(input_buffer), stdin);
            input_buffer[strlen(input_buffer) - 1] = 0;

            if (input_buffer[0] == '\\') {
                if (strcmp(input_buffer, "\\help") == 0) {
                    show_manual();
                    continue;
                }

                if (strcmp(input_buffer, "\\exit") == 0) {
                    show_msg(INFO_TYPE, "Client is shutting down...\n");
                    close(client_socket);
                    exit(EXIT_SUCCESS);
                    continue;
                }

                show_msg(ERROR_TYPE, "Unknown command. type \\help to get help.\n");
                continue;
            }
            // Send the input string to the server
            send(client_socket, create_request(input_buffer), sizeof(proto_hdr), 0);
        }

        // // Check for data from the server
        // if (FD_ISSET(client_socket, &read_fds)) {
        //     char recv_buffer[BUFFER_SIZE];
        //     ssize_t recv_size = recv(client_socket, recv_buffer, sizeof(recv_buffer), 0);

        //     if (recv_size <= 0) {
        //         show_msg(ERROR_TYPE, "Server disconnected\n");
        //         break;
        //     }

        //     // Display the converted string received from the server
        //     show_msg(INFO_TYPE, "Received from server: %.*s\n", (int)recv_size, recv_buffer);
        // }

        // Check for data from the server
        if (FD_ISSET(client_socket, &read_fds)) {
            proto_hdr recv_buffer;
            ssize_t recv_size = recv(client_socket, &recv_buffer, sizeof(proto_hdr), 0);

            if (recv_size <= 0) {
                show_msg(ERROR_TYPE, "Server disconnected\n");
                break;
            }

            // Display the converted string received from the server
            show_msg(INFO_TYPE, "[ %dms ] Received from server: %s\n"
                    , systime_delta(recv_buffer.timestamp), recv_buffer.msg);
                
            // show_msg(DEBUG_TYPE, "raw info\n");
            // print_proto_info(recv_buffer);
        }
    }

    // Close the client socket
    close(client_socket);

    return 0;
}
void show_manual() {
    printf("+----------+---------------------------------------+\n");
    printf("| Command  | Description                           |\n");
    printf("|----------|---------------------------------------|\n");
    printf("| \\exit    | Typing to quit program.               |\n");
    printf("| \\help    | Display this help message.            |\n");
    printf("+----------+---------------------------------------+\n");

}