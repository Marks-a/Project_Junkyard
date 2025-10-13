#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <sys/types.h> 
#include <ifaddrs.h> 
#include <arpa/inet.h> 

struct localMachieneInfo {
    std::string name;
    std::string ip;
    std::string subnet;
    std::string mac;
    int port;
};
void get_local_machine_info() {
    struct ifaddrs *interfaces = nullptr; 
    if (getifaddrs(&interfaces) == -1) {
        perror("getifaddrs");
        return;
    }
    for (struct ifaddrs *iface = interfaces; iface != nullptr; iface = iface->ifa_next) { 
        if (iface->ifa_addr->sa_family == AF_INET) { 
            char ip[INET_ADDRSTRLEN]; 
            inet_ntop(AF_INET, &((struct sockaddr_in *)iface->ifa_addr)->sin_addr, ip, sizeof(ip)); 
            std::cout << "Interface: " << iface->ifa_name << " IP Address: " << ip << std::endl; 
        } 
    } 
    freeifaddrs(interfaces); 
};

int main() {
    printf("Getting info...\n");
    get_local_machine_info();

    return 0;
}