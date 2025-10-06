#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORTNUM 8080
int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        perror("Socket creation failed");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr= htonl(INADDR_ANY);
    addr.sin_port = htons(PORTNUM);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(sock); return 1;
    }

    char buf[1500];
    struct sockaddr_in src;
    socklen_t srclen = sizeof(src);

    printf("UDP server listening on port %d...\n", PORTNUM);
    ssize_t n = recvfrom(sock, buf, sizeof(buf)-1, 0, (struct sockaddr*)&src, &srclen);
    if (n < 0) { perror("recvfrom"); close(sock); return 1; }
    buf[n] = '\0';
    char addrstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &src.sin_addr, addrstr, sizeof(addrstr));
    printf("Received from %s:%d: %s\n", addrstr, ntohs(src.sin_port), buf);

    close(sock);
    return 0;


}