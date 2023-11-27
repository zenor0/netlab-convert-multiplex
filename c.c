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
    }