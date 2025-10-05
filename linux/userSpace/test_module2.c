/*
Uses kernalSpace/module1.c
[ A simple hello from kernal space ]
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

int main(void)
{
    const char *dev = "/dev/module2_Testing_kernal";
    int fd = open(dev, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    char buf[512];

    printf("Listening on %s \n", dev);

    while (1) {
        int ret = poll(&pfd, 1, -1); 
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            perror("poll");
            break;
        }

        if (pfd.revents & POLLIN) {
            ssize_t n = read(fd, buf, sizeof(buf)-1);
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    continue;
                perror("read");
                break;
            } else if (n == 0) {
                printf("EOF from device\n");
                break;
            } else {
                buf[n] = '\0';
                printf("RECV: %s", buf);
                fflush(stdout);
            }
        }
    }
    close(fd);
    return 0;
}
