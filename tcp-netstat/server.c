// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    // 创建套接字
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // 设置服务器地址结构
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(12345);

    // 绑定套接字
    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));

    // 监听
    listen(server_socket, 5);

    // 接受客户端连接
    printf("等待客户端连接...\n");
    int client_socket = accept(server_socket, NULL, NULL);

    // struct sockaddr_in local_address, remote_address;
    // socklen_t addr_len = sizeof(local_address);
    // getsockname(client_socket, (struct sockaddr *)&local_address, &addr_len);
    // printf("Local address: %s:%d\n", inet_ntoa(local_address.sin_addr), ntohs(local_address.sin_port));

    // addr_len = sizeof(remote_address);
    // getpeername(client_socket, (struct sockaddr *)&remote_address, &addr_len);
    // printf("Remote address: %s:%d\n", inet_ntoa(remote_address.sin_addr), ntohs(remote_address.sin_port));

    printf("客户端已连接\n");

    // 等待一段时间，模拟TCP连接建立的不同阶段
    sleep(2);

    // 发送数据
    const char* message = "Hello, client!";
    write(client_socket, message, strlen(message));

    // 关闭连接
    printf("关闭连接\n");
    close(client_socket);
    close(server_socket);

    return 0;
}
