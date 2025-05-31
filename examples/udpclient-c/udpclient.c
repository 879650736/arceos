#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h> // 重新引入 errno.h 以便检查错误码

// 目标服务器的地址和端口
//const char *DEST_HOST = "localhost";
const char *DEST_HOST = "127.0.0.1"; 
const unsigned short DEST_PORT = 12345;

// UDP 客户端发送的消息
const char MESSAGE[] = "Hello from C UDP client!";

int main() {
    puts("Hello, C UDP client!");

    // 1. 创建 UDP 套接字
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) {
        perror("socket() error");
        return -1;
    }

    // 2. 准备 getaddrinfo 的提示结构
    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;      // 只接受 IPv4 地址
    hints.ai_socktype = SOCK_DGRAM; // 只接受 UDP 套接字类型

    // 3. 域名解析
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%u", DEST_PORT);

    int status = getaddrinfo(DEST_HOST, port_str, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerror(status));
        close(sock);
        return -1;
    }

    p = res;
    if (p == NULL) {
        fprintf(stderr, "No address found for %s\n", DEST_HOST);
        freeaddrinfo(res);
        close(sock);
        return -1;
    }

    // 打印解析到的目标 IP 地址
    char ipstr[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(((struct sockaddr_in *)(p->ai_addr))->sin_addr), ipstr, INET_ADDRSTRLEN) == NULL) {
        perror("inet_ntop() error");
        freeaddrinfo(res);
        close(sock);
        return -1;
    }
    printf("Target IP: %s, Port: %u\n", ipstr, ntohs(((struct sockaddr_in *)(p->ai_addr))->sin_port));

    // ***** 关键修改：直接调用 connect()，让它处理本地绑定 *****
    // 根据 ArceOS 的 UdpSocket::connect 实现，它会先进行本地绑定（如果未绑定），再设置对端地址。
    if (connect(sock, p->ai_addr, p->ai_addrlen) != 0) {
        perror("connect() error for UDP socket");
        freeaddrinfo(res);
        close(sock);
        return -1;
    }
    printf("UDP socket connected to target address.\n");

    // 打印客户端绑定的本地地址 (现在应该已经绑定了)
    struct sockaddr_in actual_local_addr;
    socklen_t actual_local_addr_len = sizeof(actual_local_addr);
    if (getsockname(sock, (struct sockaddr *)&actual_local_addr, &actual_local_addr_len) == 0) {
        char local_ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(actual_local_addr.sin_addr), local_ip_str, INET_ADDRSTRLEN);
        printf("UDP client bound to: %s:%u\n", local_ip_str, ntohs(actual_local_addr.sin_port));
    } else {
        perror("getsockname() error after connect");
    }


    // 4. 发送数据报
    // 由于已经 connect()，现在可以使用 send()
    printf("Sending message: '%s' to %s:%u\n", MESSAGE, ipstr, DEST_PORT);
    ssize_t bytes_sent = send(sock, MESSAGE, strlen(MESSAGE), 0); // 使用 send()

    if (bytes_sent == -1) {
        perror("send() error");
        freeaddrinfo(res);
        close(sock);
        return -1;
    }
    printf("Sent %zd bytes.\n", bytes_sent);

    // 5. 接收响应数据报 (无超时，将阻塞)
    char recv_buf[1024];
    // 由于已经 connect()，现在可以使用 recv()
    printf("Waiting for response (blocking)...\n");
    ssize_t bytes_received = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0); // 使用 recv()

    if (bytes_received == -1) {
        perror("recv() error");
        freeaddrinfo(res);
        close(sock);
        return -1;
    }

    recv_buf[bytes_received] = '\0';

    // 由于已经 connect()，recv() 不会返回发送方地址，因为发送方就是 connect() 指定的对端
    printf("Received %zd bytes: '%s'\n", bytes_received, recv_buf);

    // 6. 清理资源
    freeaddrinfo(res);
    close(sock);

    return 0;
}
