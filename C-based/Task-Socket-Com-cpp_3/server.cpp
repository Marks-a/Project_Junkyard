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
#include <sys/socket.h>
#include <sys/un.h>

#define LOCATION "/tmp/server_socket"
#define PORT 9180

struct localMachineInfo {
    std::string name;
    std::string ip;
    std::string subnet;
    std::string mac;
};
struct neighbor{
    std::string ip;
    std::string mac;
};

void get_local_machine_info(std::vector<localMachineInfo> &infoList);
void get_mac_address(const std::string &interface_name, std::string &mac_address);
// Main functions
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
            char subnet[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &((struct sockaddr_in *)iface->ifa_netmask)->sin_addr, subnet, sizeof(subnet)); 

            std::cout << "Interface: " << iface->ifa_name << " IP Address: " << ip << std::endl;
            localMachineInfo info;       
            info.name = iface->ifa_name;
            info.ip = ip;
            info.subnet = subnet;
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

int open_local_socket() {
    int local_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if(local_socket < 0) {
        perror("socket");
        return -1;
    }
    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, LOCATION, sizeof(server_addr.sun_path) - 1);
    unlink(LOCATION); // Remove any previous socket file
    int ret = bind(local_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(ret < 0) {
        perror("bind");
        close(local_socket);
        return -1;
    }
    ret = listen(local_socket, 5);
    if(ret < 0) {
        perror("listen");
        close(local_socket);
        return -1;
    }
    printf("Local socket listening at %s\n", LOCATION);
    return local_socket;
}

void main_loop(const std::vector<localMachineInfo> &infoList) {
    int local_socket = open_local_socket();
    if (local_socket < 0) {
        return;
    }

    while (true) {
        printf("Waiting for client connection...\n");
        std::string message = "Ethernet :" + infoList[0].ip + " , " + infoList[0].mac + "\n";
        int client_socket = accept(local_socket, nullptr, nullptr);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }
        
        ssize_t w = write(client_socket, message.c_str(), message.size());
        if (w < 0) {
            perror("write");
        } else {
            printf("sent %zd bytes to client\n", w);
        }

        
        close(client_socket);
        unlink(LOCATION); 
        exit(0);
    }
}
void broadcast_udp_neighbor(const localMachineInfo &info) {
   // Note: this func needs to be loop through. (1 info)
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in broadcastAddr;
    if (udp_socket < 0) {
        perror("socket");
        return;
    }
    if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, 1, sizeof(int)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, BROADCAST_ADDRESS, &broadcastAddr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }
    std::string message = "Ethernet :" + info.ip + " , " + info.mac + "\n";
    if(sendto(udp_socket, message.c_str(), message.size(), 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) < 0) {
        perror("sendto");
    } else {
        printf("Broadcasted: %s", message.c_str());
    }
    close(udp_socket);
}


    int main() {
    printf("Getting info...\n");
    std::vector<localMachineInfo> infoList;
    get_local_machine_info(infoList);

    printf("Info gathered:\n");
    for (const auto &info : infoList) {
        std::cout << "Interface: " << info.name << ", IP: " << info.ip << ", MAC: " << info.mac << ", Subnet: " << info.subnet << std::endl;
    }
    printf("Size of infoList: %zu\n", infoList.size());
    main_loop(infoList);

    return 0;
}