#ifndef ADMIN_HPP
#define ADMIN_HPP

/*
    Compile: g++ .\main.cpp -o main.exe -lwininet
    Compile 2 : 
    g++ main.cpp Utils/admin.cpp Utils/printInfo.cpp Utils/command.cpp Utils/chain_proxy.cpp -o main.exe -lws2_32 -lwininet -pthread -std=c++17
*/

#define CharBufferSize 128


#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <wininet.h>


#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <thread>



#ifdef _WIN32
    #ifndef HAVE_INET_PTON
    int inet_pton(int af, const char *src, void *dst) {
        if (af == AF_INET) {
            struct sockaddr_in sa;
            int sa_len = sizeof(sa);
            char buf[INET_ADDRSTRLEN + 1] = {0};
            strncpy(buf, src, INET_ADDRSTRLEN); // copy safely

            if (WSAStringToAddressA(buf, AF_INET, NULL, (struct sockaddr*)&sa, &sa_len) == 0) {
                memcpy(dst, &sa.sin_addr, sizeof(sa.sin_addr));
                return 1;
            }
            return 0;
        }
        return -1; // Not supported
    }
    #endif
#endif


struct Proxy {
    std::string host;
    int port;
};

bool RelayTraffic(SOCKET clientSocket, SOCKET proxySocket);
bool ForwardProxyChain(
    const std::vector<Proxy> &chain,
    const std::string &request,
    SOCKET clientSocket);
void local_Proxy(int port, const std::vector<Proxy> &chain);


bool IsRunningAsAdmin();
void print_system_info();
void show_menu();
void Com();
void proxy_settings_menu();
std::string exec_cmd(const char *cmd);


#endif
