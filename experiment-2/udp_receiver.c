#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8888
#define BUF_SIZE 1024
#define TOTAL_PACKETS 1000

int main() {
    WSADATA wsaData;
    SOCKET sockfd;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);

    char buffer[BUF_SIZE];
    int received[TOTAL_PACKETS];
    int unique_count = 0;

    memset(received, 0, sizeof(received));

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

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    printf("UDP receiver started on port %d\n", PORT);
    printf("Waiting for packets...\n");

    while (1) {
        int n = recvfrom(
            sockfd,
            buffer,
            BUF_SIZE - 1,
            0,
            (struct sockaddr *)&client_addr,
            &client_len
        );

        if (n == SOCKET_ERROR) {
            printf("recvfrom failed: %d\n", WSAGetLastError());
            continue;
        }

        buffer[n] = '\0';

        if (strcmp(buffer, "END") == 0) {
            printf("Received END\n");
            break;
        }

        int seq;

        if (sscanf(buffer, "%d", &seq) == 1) {
            if (seq >= 0 && seq < TOTAL_PACKETS) {
                if (!received[seq]) {
                    received[seq] = 1;
                    unique_count++;
                }
            }

            printf("Received packet %d from %s:%d\n",
                   seq,
                   inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port));
        }
    }

    int lost = 0;

    for (int i = 0; i < TOTAL_PACKETS; i++) {
        if (!received[i]) {
            lost++;
        }
    }

    printf("\n===== Statistics =====\n");
    printf("Expected packets : %d\n", TOTAL_PACKETS);
    printf("Received packets : %d\n", unique_count);
    printf("Lost packets     : %d\n", lost);
    printf("Packet loss rate : %.2f%%\n",
           lost * 100.0 / TOTAL_PACKETS);

    closesocket(sockfd);
    WSACleanup();

    return 0;
}