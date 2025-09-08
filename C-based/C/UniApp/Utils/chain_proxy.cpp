
#include "utils.hpp"

// struct Proxy
// {
//     std::string host;
//     int port;
// };

std::vector<Proxy> proxy_chain;

void proxy_settings_menu() {
    while (true) {
        std::cout << "\n=== Proxy Settings ===\n";
        std::cout << "[1] Show proxy chain\n";
        std::cout << "[2] Add proxy\n";
        std::cout << "[3] Delete proxy\n";
        std::cout << "[4] Back to Main Menu\n";
        std::cout << "> ";

        int choice;
        std::cin >> choice;

        if (choice == 1) {
            if (proxy_chain.empty()) {
                std::cout << "Proxy chain is empty.\n";
            } else {
                std::cout << "Current proxy chain:\n";
                for (size_t i = 0; i < proxy_chain.size(); i++) {
                    std::cout << i + 1 << ". " 
                              << proxy_chain[i].host << ":" << proxy_chain[i].port << "\n";
                }
            }
        }
        else if (choice == 2) {
            Proxy p;
            std::cout << "Enter proxy host (IP or hostname): ";
            std::cin >> p.host;
            std::cout << "Enter proxy port: ";
            std::cin >> p.port;
            proxy_chain.push_back(p);
            std::cout << "Proxy added.\n";
        }
        else if (choice == 3) {
            if (proxy_chain.empty()) {
                std::cout << "No proxies to delete.\n";
            } else {
                std::cout << "Enter index to delete (1-" << proxy_chain.size() << "): ";
                int idx;
                std::cin >> idx;
                if (idx > 0 && idx <= (int)proxy_chain.size()) {
                    proxy_chain.erase(proxy_chain.begin() + (idx - 1));
                    std::cout << "Proxy deleted.\n";
                } else {
                    std::cout << "Invalid index.\n";
                }
            }
        }
        else if (choice == 4) {
            break; // back to main menu
        }
        else {
            std::cout << "Invalid choice. Try again.\n";
        }
    }
}






bool RelayTraffic(SOCKET clientSocket, SOCKET proxySocket)
{
    fd_set readfds;
    char buffer[4096];

    while (true)
    {
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);
        FD_SET(proxySocket, &readfds);

        int maxSock = (clientSocket > proxySocket ? clientSocket : proxySocket) + 1;
        int activity = select(maxSock, &readfds, nullptr, nullptr, nullptr);

        if (activity == SOCKET_ERROR)
            break;

        if (FD_ISSET(clientSocket, &readfds))
        {
            int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytes <= 0)
                break;
            send(proxySocket, buffer, bytes, 0);
        }

        if (FD_ISSET(proxySocket, &readfds))
        {
            int bytes = recv(proxySocket, buffer, sizeof(buffer), 0);
            if (bytes <= 0)
                break;
            send(clientSocket, buffer, bytes, 0);
        }
    }

    return true;
}

bool ForwardProxyChain(
    const std::vector<Proxy> &chain,
    const std::string &request,
    SOCKET clientSocket)
{
    SOCKET sock = INVALID_SOCKET;

    // 1. Connect step-by-step through proxies
    for (size_t i = 0; i < chain.size(); i++)
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(chain[i].port);
        inet_pton(AF_INET, chain[i].host.c_str(), &addr.sin_addr);

        if (connect(sock, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
        {
            std::cerr << "Failed to connect to proxy "
                      << chain[i].host << ":" << chain[i].port << std::endl;
            closesocket(sock);
            return false;
        }

        // If not last, issue CONNECT to the next proxy
        if (i < chain.size() - 1)
        {
            std::string connectReq = "CONNECT " + chain[i + 1].host + ":" +
                                     std::to_string(chain[i + 1].port) + " HTTP/1.1\r\n\r\n";
            send(sock, connectReq.c_str(), (int)connectReq.size(), 0);
            char resp[1024];
            int len = recv(sock, resp, sizeof(resp) - 1, 0);
            if (len <= 0 || std::string(resp).find("200") == std::string::npos)
            {
                std::cerr << "CONNECT failed at " << chain[i].host << std::endl;
                closesocket(sock);
                return false;
            }
        }
    }

    // 2. Now at the last proxy → handle HTTP vs HTTPS CONNECT
    if (request.rfind("CONNECT", 0) == 0)
    {
        // It's an HTTPS CONNECT request from the browser
        // Extract host:port from CONNECT line
        std::string hostport = request.substr(8, request.find(" ", 8) - 8);

        // Send CONNECT to final target
        std::string connectReq = "CONNECT " + hostport + " HTTP/1.1\r\n\r\n";
        send(sock, connectReq.c_str(), (int)connectReq.size(), 0);

        char resp[1024];
        int len = recv(sock, resp, sizeof(resp) - 1, 0);
        if (len <= 0 || std::string(resp).find("200") == std::string::npos)
        {
            std::cerr << "CONNECT to target failed" << std::endl;
            closesocket(sock);
            return false;
        }

        // Reply to browser that tunnel is established
        std::string okResp = "HTTP/1.1 200 Connection Established\r\n\r\n";
        send(clientSocket, okResp.c_str(), (int)okResp.size(), 0);

        // Relay raw TLS traffic
        RelayTraffic(clientSocket, sock);
    }
    else
    {
        // Plain HTTP request
        send(sock, request.c_str(), (int)request.size(), 0);
        RelayTraffic(clientSocket, sock);
    }

    closesocket(sock);
    return true;
}

void local_Proxy(int port, const std::vector<Proxy> &chain)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed" << std::endl;
        return;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
    {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (bind(listenSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed" << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed" << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    std::cout << "Local proxy started at 127.0.0.1:" << port << std::endl;

    while (true)
    {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "Accept failed" << std::endl;
            break;
        }

        std::thread([clientSocket, chain]()
                    {
            char buffer[4096];
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::string request(buffer);
                ForwardProxyChain(chain, request, clientSocket);
            }
            closesocket(clientSocket); })
            .detach();
    }

    closesocket(listenSocket);
    WSACleanup();
}

/* This is function is go together with local_Proxy function*/
// bool ForwardProxyChain(
//     const std::vector<Proxy> &chain, const std::string& request, SOCKET clientSocket)
// {
//     SOCKET sock = INVALID_SOCKET;

//     for (size_t i = 0; i < chain.size(); i++)
//     {
//         // Connect to current proxy
//         sock = socket(AF_INET, SOCK_STREAM, 0);
//         sockaddr_in addr{};
//         addr.sin_family = AF_INET;
//         addr.sin_port = htons(chain[i].port);
//         inet_pton(AF_INET, chain[i].host.c_str(), &addr.sin_addr);

//         if (connect(sock, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
//         {
//             std::cerr << "Failed to connect to proxy " << chain[i].host << ":" << chain[i].port << std::endl;
//             closesocket(sock);
//             return false;
//         }

//         // For the last proxy in the chain → send the actual request
//         if (i == chain.size() - 1)
//         {
//             send(sock, request.c_str(), (int)request.size(), 0);
//         }
//         else
//         {
//             // For intermediate proxies, issue a CONNECT to the *next* proxy
//             std::string connectReq = "CONNECT " + chain[i + 1].host + ":" + std::to_string(chain[i + 1].port) + " HTTP/1.1\r\n\r\n";
//             send(sock, connectReq.c_str(), (int)connectReq.size(), 0);
//             char resp[1024];
//             int len = recv(sock, resp, sizeof(resp) - 1, 0);
//             if (len <= 0)
//             {
//                 std::cerr << "Failed to receive CONNECT response" << std::endl;
//                 closesocket(sock);
//                 return false;
//             }
//             resp[len] = '\0';
//             if (std::string(resp).find("200") == std::string::npos)
//             {
//                 std::cerr << "CONNECT failed at proxy " << chain[i].host << std::endl;
//                 closesocket(sock);
//                 return false;
//             }
//         }
//     }

//     fd_set readfds;
//     char buffer[4096];
//     while (true)
//     {
//         FD_ZERO(&readfds);
//         FD_SET(clientSocket, &readfds);
//         FD_SET(sock, &readfds);

//         int maxSock = (clientSocket > sock ? clientSocket : sock) + 1;
//         int activity = select(maxSock, &readfds, nullptr, nullptr, nullptr);

//         if (activity == SOCKET_ERROR)
//             break;

//         if (FD_ISSET(clientSocket, &readfds))
//         {
//             int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
//             if (bytes <= 0)
//                 break;
//             send(sock, buffer, bytes, 0);
//         }

//         if (FD_ISSET(sock, &readfds))
//         {
//             int bytes = recv(sock, buffer, sizeof(buffer), 0);
//             if (bytes <= 0)
//                 break;
//             send(clientSocket, buffer, bytes, 0);
//         }
//     }

//     closesocket(sock);
//     return true;
// };

// /*
//     'set_system_proxy' is needed for when the user want to chain proxy,
//     it would first 'local_Proxy' create a local proxy the by 'set_system_proxy',
//     function will direct it to it.
// */

// void local_Proxy(int port, const std::vector<Proxy>& chain)
// {
//     WSADATA wsaData;
//     if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
//     {
//         std::cerr << "WSAStartup failed" << std::endl;
//         return;
//     }

//     SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
//     if (listenSocket == INVALID_SOCKET)
//     {
//         std::cerr << "Failed to create socket" << std::endl;
//         WSACleanup();
//         return;
//     }

//     sockaddr_in serverAddr{};
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_port = htons(port);
//     inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

//     if (bind(listenSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
//     {
//         std::cerr << "Bind failed" << std::endl;
//         closesocket(listenSocket);
//         WSACleanup();
//         return;
//     }

//     if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
//     {
//         std::cerr << "Listen failed" << std::endl;
//         closesocket(listenSocket);
//         WSACleanup();
//         return;
//     }

//     std::cout << "Local proxy started at 127.0.0.1:" << port << std::endl;

//     // Accept loop (handle one client at a time for now)
//     while (true)
//     {
//         SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
//         if (clientSocket == INVALID_SOCKET)
//         {
//             std::cerr << "Accept failed" << std::endl;
//             break;
//         }

//         char buffer[4096];
//         int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
//         if (bytesReceived > 0)
//         {
//           buffer[bytesReceived] = '\0';
//             std::string request(buffer);
//             std::cout << "Request from browser:\n" << request << std::endl;

//             ForwardProxyChain(chain, request, clientSocket);

//         }

//         closesocket(clientSocket);
//     }

//     closesocket(listenSocket);
//     WSACleanup();
// }
