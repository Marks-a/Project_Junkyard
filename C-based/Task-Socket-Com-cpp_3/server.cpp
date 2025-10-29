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
#include <poll.h>
#include <chrono>

#define LOCATION "/tmp/server_socket"
#define PORT 9180
#define BUFFER_SIZE 1024
#define MAX_NO_REPLY 30
struct localMachineInfo {
    std::string name;
    std::string ip;
    std::string broadcast;
    std::string mac;
};
struct neighbor{
    std::string ip;
    std::string mac;
    std::chrono::steady_clock::time_point last_seen;
};

void get_local_machine_info(std::vector<localMachineInfo> &infoList);
void get_mac_address(const std::string &interface_name, std::string &mac_address);
sockaddr_in create_sockaddr_struct();
int setup_udp_neighbor(int &udp_socket);
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
            char broadcast[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &((struct sockaddr_in *)iface->ifa_broadaddr)->sin_addr,broadcast, sizeof(broadcast)); 

            std::cout << "Interface: " << iface->ifa_name << " IP Address: " << ip << std::endl;
            localMachineInfo info;       
            info.name = iface->ifa_name;
            info.ip = ip;
            info.broadcast = broadcast;
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
    unlink(LOCATION);
    int local_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if(local_socket < 0) {
        perror("socket");
        return -1;
    }
    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, LOCATION, sizeof(server_addr.sun_path) - 1);
    unlink(LOCATION);
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

void main_loop(std::vector<localMachineInfo> &infoList, std::vector<neighbor> &neighborList) {
    int local_socket = open_local_socket();
    if (local_socket < 0) {
        return;
    }
    int udp_socket;
    if (setup_udp_neighbor(udp_socket) < 0) {
        close(local_socket);
        return;
    }
    struct pollfd fds[2];
    fds[0].fd = local_socket;
    fds[0].events = POLLIN;
    fds[1].fd = udp_socket;
    fds[1].events = POLLIN;

    auto last_broadcast = std::chrono::steady_clock::now();
    const auto broadcast_interval = std::chrono::seconds(5);

    while (true) {
        int ret = poll(fds, 2, 1000);
        if (ret < 0) {
            perror("poll");
            break;
        }
        if(fds[0].revents & POLLIN) {
            // Need to send info in a loop.
            std::string messages;
            for(auto &nb : neighborList) {
                std::string m = "Neighbors: " + nb.ip + ", " + nb.mac + "\n";
                messages += m;
            }
            int client_socket = accept(local_socket, nullptr, nullptr);
            if (client_socket < 0) {
                    perror("accept");
                    continue;
                }
            ssize_t w = write(client_socket, messages.c_str(), messages.size());
            if (w < 0) perror("write");
            else printf("sent %zd bytes to client\n", w);
            close(client_socket);
        }
        if(fds[1].revents & POLLIN) {
            char buffer[BUFFER_SIZE];
            sockaddr_in sender{};
            socklen_t sender_len = sizeof(sender);
            ssize_t n = recvfrom(udp_socket, buffer, sizeof(buffer) - 1, 0,
                             (struct sockaddr *)&sender, &sender_len);
            if (n > 0) {
            buffer[n] = '\0';
            std::string sender_ip = inet_ntoa(sender.sin_addr);
            uint16_t sender_port = ntohs(sender.sin_port);

             printf("Received UDP from %s:%d — %s\n",
               sender_ip.c_str(), sender_port, buffer);

            bool is_own = false;
            for (const auto &info : infoList) {
            if (info.ip == sender_ip) {
                std::cout << "Received own broadcast, ignoring." << std::endl;
                is_own = false;
                break;
            }
        }
            if (is_own) continue;


            bool found = false;
            for (auto &nb : neighborList) {
            if (nb.ip == sender_ip) {
                if (std::chrono::steady_clock::now() - nb.last_seen <= std::chrono::seconds(MAX_NO_REPLY)) {
                    std::cout << "Neighbor " << nb.ip << " already in list and not timed out." << std::endl;
                } else {
                    std::cout << "Neighbor " << nb.ip << " was timed out — refreshing entry." << std::endl;
                }

                nb.last_seen = std::chrono::steady_clock::now();
                found = true;
                break;
            }
        }
        if (!found) {
            neighbor new_nb;
            new_nb.ip = sender_ip;
            size_t comma_pos = std::string(buffer).find(',');
            new_nb.mac = std::string(buffer).substr(comma_pos + 1);
            new_nb.last_seen = std::chrono::steady_clock::now();

            neighborList.push_back(new_nb);
            std::cout << "Added neighbor: " << new_nb.ip << ", " << new_nb.mac << std::endl;
        }
            std::cout << "Ended processing UDP packet." << std::endl;
        }
    }


        auto now = std::chrono::steady_clock::now();
        if (now - last_broadcast >= broadcast_interval) {
        last_broadcast = now;
        std::string m = "Broadcast: " + infoList[0].ip + " , " + infoList[0].mac + "\n";
        for (const auto &info : infoList) {
            sockaddr_in broadcastAddr{};
            broadcastAddr.sin_family = AF_INET;
            broadcastAddr.sin_port = htons(PORT);
            inet_pton(AF_INET, info.broadcast.c_str(), &broadcastAddr.sin_addr);

            ssize_t sent = sendto(udp_socket, m.c_str(), m.size(), 0,
                                  (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));
            if (sent < 0)
                perror("sendto");
            else
                printf("Broadcasted %zd bytes to %s\n", sent, info.broadcast.c_str());
        }
    }
}
    unlink(LOCATION);
    close(local_socket); 
    close(udp_socket);
    exit(0);
  
}
int setup_udp_neighbor(int &udp_socket) {
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        perror("socket");
        return -1;
    }
    int broadcast = 1;
     if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("setsockopt(SO_BROADCAST)");
        close(udp_socket);
        return -1;
    }

    int reuse = 1;
    if (setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
        close(udp_socket);
        return -1;
    }

    sockaddr_in localAddr = create_sockaddr_struct();
    localAddr.sin_addr.s_addr = htons(INADDR_ANY);

   if (bind(udp_socket, (sockaddr *)&localAddr, sizeof(localAddr)) < 0) {
        perror("bind");
        close(udp_socket);
        return -1;
    }
    
    return 0;
}
sockaddr_in create_sockaddr_struct() {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    //addr.sin_addr.s_addr = INADDR_ANY;
    return addr;
}
int main() {
    printf("Getting info...\n");
    std::vector<localMachineInfo> infoList;
    std::vector<neighbor> neighborList;
    get_local_machine_info(infoList);

    printf("Info gathered:\n");
    for (const auto &info : infoList) {
        std::cout << "Interface: " << info.name << ", IP: " << info.ip << ", MAC: " << info.mac << ",broadcast: " << info.broadcast << std::endl;
    }
    printf("Size of infoList: %zu\n", infoList.size());
    main_loop(infoList, neighborList);
    return 0;
}