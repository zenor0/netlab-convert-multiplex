// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    // 创建套接字
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // 设置服务器地址结构
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(12345);

    // 连接服务器
    connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    printf("已连接到服务器\n");

    // 等待一段时间，模拟TCP连接建立的不同阶段
    sleep(2);

    // 接收数据
    char buffer[1024];
    read(client_socket, buffer, sizeof(buffer));
    printf("从服务器接收到的数据: %s\n", buffer);

    // 关闭连接
    close(client_socket);

    return 0;
}
