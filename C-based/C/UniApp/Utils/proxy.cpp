#include "utils.hpp"

/*
    This is for proxy, chain proxy (1 only proxy) and vpn related functions of the UniApp
*/

// Route all browser traffic through a proxy server
// For cmd check : netsh winhttp show proxy
bool set_system_proxy(
    const std::wstring &proxyAddress)
{
    INTERNET_PER_CONN_OPTION_LIST option_list;
    INTERNET_PER_CONN_OPTION options[3];
    DWORD listsize = sizeof(option_list);

    // Conf
    options[0].dwOption = INTERNET_PER_CONN_FLAGS;
    options[0].Value.dwValue = PROXY_TYPE_PROXY;

    options[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    options[1].Value.pszValue = const_cast<LPWSTR>(proxyAddress.c_str());

    options[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    options[2].Value.pszValue = L"<local>";

    // List fill
    option_list.dwSize = sizeof(option_list);
    option_list.pszConnection = NULL;
    option_list.dwOptionCount = 3;
    option_list.dwOptionError = 0;
    option_list.pOptions = options;

    // Apply proxy settings
    if (InternetSetOption(NULL, INTERNET_OPTION_PROXY, &option_list, listsize))
    {
        std::cerr << "Failed to set proxy. Error: " << GetLastError() << std::endl;
        return false
    }
    InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0);

    std::wcout << L"Proxy set to " << proxyAddress << std::endl;
    return true;
};



bool disable_system_proxy()
{
    INTERNET_PER_CONN_OPTION_LIST optionList;
    INTERNET_PER_CONN_OPTION option;
    DWORD listSize = sizeof(optionList);

    option.dwOption = INTERNET_PER_CONN_FLAGS;
    option.Value.dwValue = PROXY_TYPE_DIRECT; // Direct = no proxy

    optionList.dwSize = sizeof(optionList);
    optionList.pszConnection = NULL;
    optionList.dwOptionCount = 1;
    optionList.dwOptionError = 0;
    optionList.pOptions = &option;

    if (!InternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &optionList, listSize))
    {
        std::cerr << "Failed to disable proxy. Error: " << GetLastError() << std::endl;
        return false;
    }

    InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0);

    std::cout << "Proxy disabled." << std::endl;
    return true;
}


// Do not need? Prorotype
bool connect_to_proxy(
    /* The proxy server */
    const std::string &proxy_address,
    int proxy_port,
    /* The target to connect through the proxy */
    const std::string &target_host,
    int target_port,
    /* connections socket to talk to the target */
    SOCKET &out_socket)
{
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed." << std::endl;
        return false;
    }
    /* AF_INET -> IPv4, AF_INET6 -> IPv6 */
    out_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (out_socket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return false;
    }

    // Setup
    sockaddr_in proxy_addr;
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(proxy_port);
    proxy_addr.sin_addr.s_addr = inet_addr(proxy_address.c_str());
    if (proxy_addr.sin_addr.s_addr == INADDR_NONE)
    {
        std::cerr << "Invalid proxy address." << std::endl;
        closesocket(out_socket);
        WSACleanup();
        return false;
    }
    // Connection
    try
    {
        if (connect(out_socket, (sockaddr *)&proxy_addr, sizeof(proxy_addr)) == SOCKET_ERROR)
        {
            std::cerr << "Failed to connect to proxy." << std::endl;
            closesocket(out_socket);
            WSACleanup();
            return false;
        }
    }
    catch (...)
    {
        std::cerr << "Exception occurred while connecting to proxy." << std::endl;
        closesocket(out_socket);
        WSACleanup();
        return false;
    }

    return true;
}