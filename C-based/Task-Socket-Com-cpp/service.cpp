/*
    Best use case: Unix domain socket
    1. Dedicated to start broadcasting the info:
        IP, Mac address to others (Ip->Key, Mac->Value?)
        Expires: 30 sec (Maybe separate in step 4)
    2. Runs continuously.
    3. Listens for broadcasting info from step 1
    4. Keeps table, with IP,Mac and expire time
    5. Cooperates with cli.cpp with step 4 info

    [! Must not use other services to achieve this]
    [! Without Threads/Exception ]
    [! May use daemon for this service (Requirment: manually exec) ]
    [! Might add setpriority() ]

    [ Maybe better option: Send just a socket without a string, but no MAC ]
    [ cli.cpp will may additional need for struct sockaddr_ll to read MAC from layer 2 ]

*/

#include <iostream>
#include <string> // Maybe to send info in a string format
#include <cstring>
#include <ctime> // ???
#include <stdio.h>
#include <vector>
#include <algorithm>

// Linux specific headers
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>

#define PORT 54321
#define DOMAIN_SOCKET_PATH "/tmp/service_socket.sock"
#define TIMEOUT 30
#define BROADCAST_INTERVAL 10
#define MAC_LENGTH 6

struct Neighbor
{
    std::string name;
    std::string ip;
    std::string mac;
    time_t lastSeen;
};

struct LocalInfo
{
    std::string name;
    std::string ip;
    std::string mac;
    std::string bcast;
    std::string netmask;

    bool operator==(const LocalInfo &other) const
    {
        return name == other.name && ip == other.ip && mac == other.mac &&
               bcast == other.bcast && netmask == other.netmask;
    }
};

void check_local_info();
void delete_expired_neighbors(std::vector<Neighbor> &neighbors, const time_t &now);
bool is_same_sub(const std::string &sender_ip_str, const LocalInfo &iface);

std::vector<LocalInfo> get_info()
{
    std::vector<LocalInfo> result;
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return result;
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
            continue;
        // IPv4 (no 2.0.0.0 ip's)
        if (ifa->ifa_addr->sa_family != AF_INET)
            continue;
        // Skip loopback (Skip 'lo')
        if (ifa->ifa_flags & IFF_LOOPBACK)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;
        LocalInfo info;
        info.name = ifa->ifa_name;
        // IP
        char ipbuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr,
                  ipbuf, sizeof(ipbuf));
        info.ip = ipbuf;
        /* Mac
        Uses socket, cuz AF_PACKET for some reason did not give the address */
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd >= 0)
        {
            struct ifreq ifr;
            memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);
            if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0)
            {
                unsigned char *hw = (unsigned char *)ifr.ifr_hwaddr.sa_data;
                char macbuf[18];
                snprintf(macbuf, sizeof(macbuf),
                         "%02x:%02x:%02x:%02x:%02x:%02x",
                         hw[0], hw[1], hw[2], hw[3], hw[4], hw[5]);
                info.mac = macbuf;
            }
            close(fd);
        }

        // Broadcast address
        if (ifa->ifa_flags & IFF_BROADCAST && ifa->ifa_broadaddr)
        {
            char bcastbuf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_broadaddr)->sin_addr,
                      bcastbuf, sizeof(bcastbuf));
            info.bcast = bcastbuf;
        }
        // Netmask
        if (ifa->ifa_netmask)
        {
            char netmaskbuf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr,
                      netmaskbuf, sizeof(netmaskbuf));
            info.netmask = netmaskbuf;
        }

        result.push_back(info);
    }

    freeifaddrs(ifaddr);
    return result;
}

void send_hello(bool &debug, int sock, const LocalInfo &info)
{
    if (info.bcast.empty())
        return;
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, info.bcast.c_str(), &addr.sin_addr);

    char msg[128];
    snprintf(msg, sizeof(msg), "HELLO %s %s %s", info.name.c_str(), info.ip.c_str(), info.mac.c_str());

    if (debug)
    {
        printf("Sending HELLO: %s %s %s -> %s\n",
               info.name.c_str(), info.ip.c_str(), info.mac.c_str(), info.bcast.c_str());
    }
    sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&addr, sizeof(addr));
}

void receive_hello(bool &debug, int sock, std::vector<Neighbor> &neighbors, const std::vector<LocalInfo> &self, time_t &now)
{
    char buf[256];
    struct sockaddr_in src;
    socklen_t srclen = sizeof(src);

    int n = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&src, &srclen);
    if (n <= 0)
        return;
    buf[n] = '\0';
    std::string name, ip, mac;
    char namebuf[64], ipbuf[64], macbuf[32];
    if (sscanf(buf, "HELLO %63s %63s %31s", namebuf, ipbuf, macbuf) < 3)
        return;
    name = namebuf;
    ip = ipbuf;
    mac = macbuf;

    // Skip our own broadcasts
    for (auto &iface : self)
    {
        if (iface.mac == mac)
            return;
    }

    // From the actual sender IP (May pass src.sin_addr to 'is_same_sub')
    char sender_ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &src.sin_addr, sender_ip_str, sizeof(sender_ip_str));

    bool in_same_net = false;
    // May need improvement, to improve eff
    // Just send src.sin_addr and LocalInfo have IP/Net store as in_addr_t / in_addr
    for (const auto &iface : self)
    {
        if (!iface.netmask.empty() && is_same_sub(sender_ip_str, iface))
        {
            in_same_net = true;
            break;
        }
    }
    if (!in_same_net)
        return;

    if (debug)
    {
        printf("Received HELLO from %s %s %s\n", name.c_str(), ip.c_str(), mac.c_str());
    }
    bool found = false;
    for (auto &nb : neighbors)
    {
        if (nb.mac == mac)
        {
            nb.lastSeen = now;
            found = true;
            break;
        }
    }
    if (!found)
    {
        Neighbor nb{name, ip, mac, now};
        neighbors.push_back(nb);
    }
}
bool is_same_sub(const std::string &sender_ip_str, const LocalInfo &iface)
{
    in_addr sender_ip;
    if (inet_pton(AF_INET, sender_ip_str.c_str(), &sender_ip) != 1)
        return false;

    // for (const auto &iface : self)
    // {
    in_addr iface_ip, iface_mask;
    inet_pton(AF_INET, iface.ip.c_str(), &iface_ip);
    inet_pton(AF_INET, iface.netmask.c_str(), &iface_mask);

    if ((sender_ip.s_addr & iface_mask.s_addr) == (iface_ip.s_addr & iface_mask.s_addr)) 
        return true;
    //}
    return false;
}
void myInfo_update(bool &debug, std::vector<LocalInfo> &myInfo)
{
    std::vector<LocalInfo> newInfo = get_info();
    if (newInfo == myInfo)
        return;
    myInfo = newInfo;
    for (auto &iface : myInfo)
    {
        if (iface.bcast.empty())
            continue;
        if (debug)
        {
            printf("Updated info: %s %s %s %s\n", iface.name.c_str(), iface.ip.c_str(), iface.mac.c_str(), iface.bcast.c_str());
        }
    }
}

void snd_rec_msg(bool &debug)
{
    std::vector<Neighbor> neighbors;
    std::vector<LocalInfo> myInfo = get_info();

    /* Open socket for sending info IP4 */
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return;
    }

    /* Unix domain socket */
    unlink(DOMAIN_SOCKET_PATH);
    int unix_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (unix_fd < 0)
    {
        perror("socket(AF_UNIX)");
        return;
    }

    /* For NC allow other to listen to the same port IP4 */
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
    }

#ifdef SO_REUSEPORT
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(SO_REUSEPORT) failed");
    }
#endif
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(SO_BROADCAST) failed");
    }

    /* Unix domain socket */
    struct sockaddr_un unix_addr{};
    unix_addr.sun_family = AF_UNIX;
    strncpy(unix_addr.sun_path, DOMAIN_SOCKET_PATH, sizeof(unix_addr.sun_path) - 1);
    if (bind(unix_fd, (struct sockaddr *)&unix_addr, sizeof(unix_addr)) < 0)
    {
        perror("bind(AF_UNIX)");
        close(unix_fd);
        return;
    }
    if (listen(unix_fd, 5) < 0)
    {
        perror("listen");
        close(unix_fd);
        return;
    }

    /* IP4 SOCKET */
    struct sockaddr_in recv_addr{};
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_addr.s_addr = INADDR_ANY;
    recv_addr.sin_port = htons(PORT);
    if (bind(sock, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0)
    {
        perror("bind failed");
        close(sock);
        return;
    }

    fcntl(sock, F_SETFL, O_NONBLOCK);
    time_t last_hello = 0;

    /* Main while loop */
    std::cout << "Service running" << std::endl;
    while (1)
    {
        time_t now = time(NULL);
        /* socket */
        fd_set readfds;
        FD_ZERO(&readfds);

        FD_SET(sock, &readfds);
        FD_SET(unix_fd, &readfds);
        int max_fd = std::max(sock, unix_fd);

        struct timeval tv{1, 0};
        int rv = select(max_fd + 1, &readfds, NULL, NULL, &tv);
        if (rv > 0 && FD_ISSET(sock, &readfds))
        {
            receive_hello(debug, sock, neighbors, myInfo, now);
        }
        if (now - last_hello >= BROADCAST_INTERVAL)
        {
            myInfo_update(debug, myInfo);
            for (const auto &iface : myInfo)
            {
                if (iface.bcast.empty())
                    continue;
                send_hello(debug, sock, iface);
            }
            last_hello = now;
        }

        delete_expired_neighbors(neighbors, now);

        // /*Unix  code*/
        if (FD_ISSET(unix_fd, &readfds))
        {

            int client_fd = accept(unix_fd, nullptr, nullptr);
            if (client_fd < 0)
            {
                perror("accept");
                continue;
            }
            // respond [ Can be a seperate function ]
            std::string output;
            if (neighbors.empty())
            {
                output = "No active neighbor\n";
            }
            else
            {
                for (auto &n : neighbors)
                {
                    output += n.name + " " + n.ip + " " + n.mac + "\n";
                }
            }
            write(client_fd, output.c_str(), output.size());
            close(client_fd);
        }
    }

    close(sock);
    close(unix_fd);
    unlink(DOMAIN_SOCKET_PATH);
}

int main(int argc, char *argv[])
{
    bool debug = false;
    std::cout << "Starting service..." << std::endl;
    if (argc > 1 && argv[1] == std::string("-d"))
    {
        debug = true;
    }
    snd_rec_msg(debug);
    return 0;
}

void check_local_info()
{
    std::cout << "#####################" << std::endl;
    std::vector<LocalInfo> infoList = get_info();
    for (const auto &info : infoList)
    {

        std::cout << info.name << " " << info.ip << " " << info.mac << std::endl;
    }
}

void delete_expired_neighbors(std::vector<Neighbor> &neighbors, const time_t &now)
{
    neighbors.erase(std::remove_if(neighbors.begin(), neighbors.end(),
                                   [now](const Neighbor &nb)
                                   { return difftime(now, nb.lastSeen) > TIMEOUT; }),
                    neighbors.end());
}
