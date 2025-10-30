#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "/tmp/server_socket"
#define BUFFER_SIZE 480

int main()
{
    int                 ret;
    int                 data_socket;
    ssize_t             r;
    struct sockaddr_un  addr;
    char                buffer[BUFFER_SIZE];

    data_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (data_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);
    ret = connect(data_socket, (const struct sockaddr *) &addr, sizeof(addr));
    if (ret == -1) {
        fprintf(stderr, "The server is down.\n");
        exit(EXIT_FAILURE);
    }

    r = read(data_socket, buffer, sizeof(buffer));
    if (r == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    buffer[sizeof(buffer) - 1] = 0;
    printf("Result = %s\n", buffer);
    close(data_socket);
    exit(EXIT_SUCCESS);
}