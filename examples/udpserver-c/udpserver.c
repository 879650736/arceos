#include <stdio.h>      // 标准输入输出，用于 puts, printf, perror
#include <string.h>     // 字符串操作，用于 strlen
#include <unistd.h>     // POSIX 系统 API，用于 close
#include <arpa/inet.h>  // Internet 地址转换函数，用于 inet_pton, htons
#include <netinet/in.h> // Internet 地址结构，用于 sockaddr_in
#include <sys/socket.h> // 套接字函数，用于 socket, bind, recvfrom, sendto

// 服务器监听的 IP 地址和端口
const char *LOCAL_IP = "0.0.0.0"; // 监听所有可用网络接口
const unsigned short LOCAL_PORT = 12345;

// UDP 服务器的响应内容
const char UDP_RESPONSE_CONTENT[] = "Hello from C UDP Server!";

int main() {
    puts("Hello, C UDP server!");

    struct sockaddr_in local_addr, remote_addr;
    socklen_t remote_addr_len = sizeof(remote_addr); // 注意这里是 socklen_t 类型

    // 1. 创建 UDP 套接字
    // AF_INET: IPv4
    // SOCK_DGRAM: 数据报套接字 (UDP)
    // IPPROTO_UDP: UDP 协议
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) {
        perror("socket() error");
        return -1;
    }

    // 2. 准备本地地址结构
    memset(&local_addr, 0, sizeof(local_addr)); // 清零结构体
    local_addr.sin_family = AF_INET;
    // 将 IP 地址字符串转换为网络字节序的二进制形式
    if (inet_pton(AF_INET, LOCAL_IP, &(local_addr.sin_addr)) != 1) {
        perror("inet_pton() error");
        close(sock); // 发生错误时关闭套接字
        return -1;
    }
    // 设置端口号，并转换为网络字节序
    local_addr.sin_port = htons(LOCAL_PORT);

    // 3. 绑定套接字到本地地址和端口
    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) != 0) {
        perror("bind() error");
        close(sock); // 发生错误时关闭套接字
        return -1;
    }

    printf("UDP server listening on %s:%u\n", LOCAL_IP, LOCAL_PORT);

    char recv_buf[1024]; // 接收缓冲区
    int packet_count = 0; // 计数器

    // 4. 进入循环，接收和发送数据报
    for (;;) {
        // 接收数据报
        // recvfrom() 会阻塞直到接收到一个数据报
        // 它会填充 remote_addr 为发送方的地址
        ssize_t bytes_received = recvfrom(sock, recv_buf, sizeof(recv_buf) - 1, 0,
                                          (struct sockaddr *)&remote_addr, &remote_addr_len);

        if (bytes_received == -1) {
            perror("recvfrom() error");
            // 在实际服务器中，这里可能不会直接退出，而是继续循环或进行更复杂的错误处理
            continue; // 继续下一次循环，尝试接收下一个数据报
        }

        recv_buf[bytes_received] = '\0'; // 确保接收到的数据是 null 终止的字符串

        // 将发送方 IP 地址转换为可读字符串
        char remote_ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(remote_addr.sin_addr), remote_ip_str, INET_ADDRSTRLEN);
        unsigned short remote_port = ntohs(remote_addr.sin_port);

        printf("Packet %d: Received %zd bytes from %s:%u: '%s'\n",
               packet_count, bytes_received, remote_ip_str, remote_port, recv_buf);

        // 发送响应数据报
        // sendto() 将数据发送回 remote_addr (发送方)
        ssize_t bytes_sent = sendto(sock, UDP_RESPONSE_CONTENT, strlen(UDP_RESPONSE_CONTENT), 0,
                                    (struct sockaddr *)&remote_addr, remote_addr_len);

        if (bytes_sent == -1) {
            perror("sendto() error");
            // 同样，这里可能不会直接退出
            continue; // 继续下一次循环
        }

        printf("Packet %d: Sent %zd bytes response to %s:%u\n",
               packet_count, bytes_sent, remote_ip_str, remote_port);

        packet_count++;
    }

    // 5. 关闭套接字 (理论上不会执行，因为是无限循环)
    if (close(sock) == -1) {
        perror("close() error");
        return -1;
    }

    return 0;
}
