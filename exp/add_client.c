#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <server1_ip> <server1_port> [<server2_ip> <server2_port> ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Loop through the provided server addresses and ports
    for (int i = 1; i < argc; i += 2)
    {
        const char *server_ip = argv[i];
        const int server_port = atoi(argv[i + 1]);

        // Create socket
        int client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1)
        {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        // Set up server address
        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(server_port);
        if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0)
        {
            perror("Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
        }

        // Connect to the server
        if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
        {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }

        srand((unsigned)time(NULL));
        // Send random numbers to the server
        int num1 = rand() % 100;
        int num2 = rand() % 100;
        printf("Sending numbers %d and %d to server %s:%d\n", num1, num2, server_ip, server_port);

        send(client_socket, &num1, sizeof(num1), 0);
        send(client_socket, &num2, sizeof(num2), 0);

        // Receive and display the server's response
        int result;
        recv(client_socket, &result, sizeof(result), 0);
        printf("Server %s:%d response: %d\n", server_ip, server_port, result);

        // Close the socket
        close(client_socket);
    }

    return 0;
}
