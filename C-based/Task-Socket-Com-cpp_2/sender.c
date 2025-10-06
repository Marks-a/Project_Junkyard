#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORTNUM 8080
int main(int argc, char **argv) {
    if (argc != 2) { printf("Usage: %s <dest-ip>\n", argv[0]); return 1; }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(PORTNUM);
    if (inet_pton(AF_INET, argv[1], &dest.sin_addr) != 1) {
        fprintf(stderr,"Invalid address\n"); close(sock); return 1;
    }

    const char *msg = "Hello";
    ssize_t sent = sendto(sock, msg, strlen(msg), 0, (struct sockaddr*)&dest, sizeof(dest));
    if (sent < 0) { perror("sendto"); close(sock); return 1; }

    printf("Sent \"%s\" to %s:%d\n", msg, argv[1], PORTNUM);
    close(sock);
    return 0;
}
