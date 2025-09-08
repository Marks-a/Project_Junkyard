/*
    Using: Unix domain socket
    1. Connects to service.cpp to receive broadcasting info.
    2. Displays the info received from service.cpp.
    [Info must display, IP, Mac and expire time]
    [If no active neigboar, display "No active neighbor"]
    [! Without Threads/Exception ]

*/

#include <iostream>
#include <cstring>
#include <vector>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define DOMAIN_SOCKET_PATH "/tmp/service_socket.sock"
#define PORT 54321
#define BUFFER_SIZE 1024
#define BUFFER_LIMIT 4084

int main()
{

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket related err");
        return 1;
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, DOMAIN_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("No Connection (Service is not running?)");
        close(sock);
        return 1;
    }

    std::vector<char> buffer(BUFFER_SIZE);
    ssize_t n = read(sock, buffer.data(), buffer.size() - 1);
    if (n < 0)
    {
        perror("Reading related err");
        close(sock);
        return 1;
    }
    // ssize_t n = read(sock, buffer, sizeof(buffer) - 1);
    // if (n < 0 || n > BUFFER_LIMIT)
    // {
    //     perror("Reading related err or buffer overflow");
    //     close(sock);
    //     return 1;
    // }
    // if (n > BUFFER_SIZE && n < BUFFER_LIMIT)
    // {
    //     buffer.resize(n);
    // }

    buffer[n] = '\0';
    std::cout << "Neighbors list:\n"
              << buffer.data();

    close(sock);
    return 0;
}