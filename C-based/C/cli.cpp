/*
    Using: Unix domain socket
    1. Connects to service.cpp to receive broadcasting info.
    2. Displays the info received from service.cpp.
    [Info must display, IP, Mac and expire time]
    [If no active neigboar, display "No active neighbor"]
    [! Without Threads/Exception ]

*/

#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

#define DOMAIN_SOCKET_PATH "/tmp/service_socket.sock"
#define PORT 54321

int main() {
    
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, DOMAIN_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    char buffer[1024];
    ssize_t n = read(sock, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        perror("read");
        close(sock);
        return 1;
    }
    
    buffer[n] = '\0';
    std::cout << "Received data:\n" << buffer;

    close(sock);
    return 0;
}