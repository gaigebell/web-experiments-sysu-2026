#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8888
#define TOTAL_PACKETS 1000
#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    SOCKET sockfd;
    struct sockaddr_in receiver_addr;

    char buffer[BUF_SIZE];

    if (argc < 2) {
        printf("Usage: %s <receiver_ip> [interval_ms]\n", argv[0]);
        printf("Example: %s 192.168.1.20 1\n", argv[0]);
        return 1;
    }

    const char *receiver_ip = argv[1];
    int interval_ms = 1;

    if (argc >= 3) {
        interval_ms = atoi(argv[2]);
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(PORT);

    receiver_addr.sin_addr.s_addr = inet_addr(receiver_ip);

    if (receiver_addr.sin_addr.s_addr == INADDR_NONE) {
        printf("Invalid receiver IP\n");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    printf("Sending %d UDP packets to %s:%d\n",
           TOTAL_PACKETS,
           receiver_ip,
           PORT);

    printf("Interval = %d ms\n", interval_ms);

    for (int i = 0; i < TOTAL_PACKETS; i++) {
        snprintf(buffer,
                 sizeof(buffer),
                 "%d UDP packet number %d",
                 i,
                 i);

        int n = sendto(
            sockfd,
            buffer,
            (int)strlen(buffer),
            0,
            (struct sockaddr *)&receiver_addr,
            sizeof(receiver_addr)
        );

        if (n == SOCKET_ERROR) {
            printf("sendto failed at packet %d: %d\n",
                   i,
                   WSAGetLastError());
        }

        printf("Sent packet %d\n", i);

        if (interval_ms > 0) {
            Sleep(interval_ms);
        }
    }

    sendto(
        sockfd,
        "END",
        3,
        0,
        (struct sockaddr *)&receiver_addr,
        sizeof(receiver_addr)
    );

    printf("Finished sending packets.\n");

    closesocket(sockfd);
    WSACleanup();

    return 0;
}