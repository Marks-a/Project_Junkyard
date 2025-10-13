#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <sys/types.h> 
#include <ifaddrs.h> 
#include <arpa/inet.h> 
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

struct localMachineInfo {
    std::string name;
    std::string ip;
    std::string subnet;
    std::string mac;
    int port;
};

void get_local_machine_info(std::vector<localMachineInfo> &infoList);
void get_mac_address(const std::string &interface_name, std::string &mac_address);

void get_local_machine_info(std::vector<localMachineInfo> &infoList) {
    struct ifaddrs *interfaces = nullptr; 
    if (getifaddrs(&interfaces) == -1) {
        perror("getifaddrs");
        return;
    }
    for (struct ifaddrs *iface = interfaces; iface != nullptr; iface = iface->ifa_next) { 
        if (iface->ifa_addr->sa_family == AF_INET) {
            if(iface->ifa_flags & IFF_LOOPBACK) continue;
            char ip[INET_ADDRSTRLEN]; 
            inet_ntop(AF_INET, &((struct sockaddr_in *)iface->ifa_addr)->sin_addr, ip, sizeof(ip)); 
            std::cout << "Interface: " << iface->ifa_name << " IP Address: " << ip << std::endl;
            localMachineInfo info;       
            info.name = iface->ifa_name;
            info.ip = ip;
            get_mac_address(iface->ifa_name, info.mac);
            infoList.push_back(info);
        } 
    } 
    freeifaddrs(interfaces); 
};
void get_mac_address(const std::string &interface_name , std::string &mac_address) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        return;
    }
    struct ifreq ifr;
    strncpy(ifr.ifr_name, interface_name.c_str(), IFNAMSIZ-1);
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
        perror("ioctl");
        close(fd);
        return;
    }
    close(fd);
    unsigned char *mac = reinterpret_cast<unsigned char *>(ifr.ifr_hwaddr.sa_data);
    printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        char mac_str[18];
        snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        mac_address = mac_str;
    }

int main() {
    printf("Getting info...\n");
    std::vector<localMachineInfo> infoList;
    get_local_machine_info(infoList);

    printf("Info gathered:\n");
    for (const auto &info : infoList) {
        std::cout << "Interface: " << info.name << ", IP: " << info.ip << ", MAC: " << info.mac << std::endl;
    }

    return 0;
}