/*
    scans a given range of ports
    checks BOTH TCP and UDP on each port
    Prints:  <port>/<proto>  OPEN, CLOSED, FILTERED and service name
    Compile:  gcc ./PortScan.c -o scanner -lws2_32
              scanner.exe IP start-port end-port
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#define TCP_TIMEOUT_MS   500     
#define UDP_TIMEOUT_MS  1000     
#define MAX_THREADS    1000    

typedef struct {
    const char *ip;
    int port;
} ThreadArgs;

const char *Serv_Name(int port, const char *proto) {
    struct servent *se = getservbyport(htons(port), proto);
    return se ? se->s_name : "";
}


void tcp(const char *ip, int port) {
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) return;

    u_long nb = 1;
    ioctlsocket(s, FIONBIO, &nb);

    struct sockaddr_in target = {0};
    target.sin_family = AF_INET;
    target.sin_port   = htons(port);
    target.sin_addr.s_addr = inet_addr(ip);

    connect(s, (struct sockaddr*)&target, sizeof(target)); 

    fd_set wfds; FD_ZERO(&wfds); FD_SET(s, &wfds);
    struct timeval tv = { TCP_TIMEOUT_MS / 1000, (TCP_TIMEOUT_MS % 1000) * 1000 };

    int sel = select(0, NULL, &wfds, NULL, &tv);
    if (sel > 0 && FD_ISSET(s, &wfds)) {
        printf("%5d - tcp OPEN (%s)\n", port, Serv_Name(port,"tcp"));
    }
    closesocket(s);
}

void udp(const char *ip, int port) {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) return;
    DWORD to = UDP_TIMEOUT_MS;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&to, sizeof(to));

    struct sockaddr_in target = {0};
    target.sin_family = AF_INET;
    target.sin_port   = htons(port);
    target.sin_addr.s_addr = inet_addr(ip);

    char pkt[1] = {0};
    sendto(s, pkt, sizeof(pkt), 0, (struct sockaddr*)&target, sizeof(target));

    char buf[64];
    int   len = sizeof(target);
    int r = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&target, &len);

    if (r >= 0) {
        printf("%5d - udp OPEN (%s)\n", port, Serv_Name(port,"udp"));
    } else {
        int e = WSAGetLastError();
        if (e == WSAETIMEDOUT) {           
            printf("%5d - udp OPEN - FILTERED (%s)\n", port, Serv_Name(port,"udp"));
        } 
    }
    closesocket(s);
}

DWORD WINAPI thread(LPVOID arg) {
    ThreadArgs *t = (ThreadArgs*)arg;
    tcp(t->ip, t->port);
    udp(t->ip, t->port);
    free(t);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("%s IP start_port end_port\n", argv[0]);
        return 1;
    }
    const char *ip      = argv[1];
    int start_port      = atoi(argv[2]);
    int end_port        = atoi(argv[3]);

    /* Winsock init */
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr,"WSAStartup failed.\n");
        return 1;
    }

    printf("Scanning %s from %d to %d (TCP & UDP)…\n\n", ip, start_port, end_port);

    HANDLE sem = CreateSemaphore(NULL, MAX_THREADS, MAX_THREADS, NULL);

    for (int p = start_port; p <= end_port; ++p) {
        WaitForSingleObject(sem, INFINITE);      // acquire slot
        ThreadArgs *ta = malloc(sizeof(ThreadArgs));
        ta->ip = ip; ta->port = p;
        HANDLE th = CreateThread(NULL, 0, thread, ta, 0, NULL);
        CloseHandle(th);                        
        ReleaseSemaphore(sem, 1, NULL);          
    }

    Sleep(UDP_TIMEOUT_MS + 500);
    WSACleanup();
    return 0;
}
