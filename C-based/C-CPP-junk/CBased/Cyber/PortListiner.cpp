#ifndef _WIN32_WINNT               // Vista / Server 2008 or newer
#define _WIN32_WINNT 0x0600
#endif
#ifndef WINVER                      // (older SDKs look at WINVER, too)
#define WINVER 0x0600
#endif
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>              // must come first
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <processthreadsapi.h>     // optional, but makes the intent clear
#include <psapi.h>
#include <cstdio>
#include <cstdlib>
#include <string>


#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "psapi.lib")

std::string GetProcessName(DWORD pid)
{
    char name[MAX_PATH] = "<unknown>";
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
                           FALSE, pid);
    if (h)
    {
        DWORD size = sizeof(name);
        if (QueryFullProcessImageNameA(h, 0, name, &size))
        {
            // strip full path → basename only
            std::string full(name);
            return full.substr(full.find_last_of("\\/") + 1);
        }
        CloseHandle(h);
    }
    return name;
}

void PrintTcpTable()
{
    DWORD len = 0;
    if (GetExtendedTcpTable(nullptr, &len, FALSE,
                            AF_INET, TCP_TABLE_OWNER_PID_ALL, 0)
        == ERROR_INSUFFICIENT_BUFFER)
    {
        auto *buf = (PMIB_TCPTABLE_OWNER_PID)std::malloc(len);
        if (GetExtendedTcpTable(buf, &len, FALSE,
                                AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR)
        {
            for (DWORD i = 0; i < buf->dwNumEntries; ++i)
            {
                MIB_TCPROW_OWNER_PID &row = buf->table[i];

                in_addr localAddr{ row.dwLocalAddr };
                in_addr remoteAddr{ row.dwRemoteAddr };

                printf("%6u  %-20s  %-5s  %s:%u  %s:%u\n",
                       row.dwOwningPid,
                       GetProcessName(row.dwOwningPid).c_str(),
                       "TCP",
                       inet_ntoa(localAddr), ntohs((u_short)row.dwLocalPort),
                       inet_ntoa(remoteAddr), ntohs((u_short)row.dwRemotePort));
            }
        }
        std::free(buf);
    }
}

void PrintUdpTable()
{
    DWORD len = 0;
    if (GetExtendedUdpTable(nullptr, &len, FALSE,
                            AF_INET, UDP_TABLE_OWNER_PID, 0)
        == ERROR_INSUFFICIENT_BUFFER)
    {
        auto *buf = (PMIB_UDPTABLE_OWNER_PID)std::malloc(len);
        if (GetExtendedUdpTable(buf, &len, FALSE,
                                AF_INET, UDP_TABLE_OWNER_PID, 0) == NO_ERROR)
        {
            for (DWORD i = 0; i < buf->dwNumEntries; ++i)
            {
                MIB_UDPROW_OWNER_PID &row = buf->table[i];
                in_addr localAddr{ row.dwLocalAddr };

                printf("%6u  %-20s  %-5s  %s:%u  %s\n",
                       row.dwOwningPid,
                       GetProcessName(row.dwOwningPid).c_str(),
                       "UDP",
                       inet_ntoa(localAddr), ntohs((u_short)row.dwLocalPort),
                       "0.0.0.0:0");   // UDP has no connected remote end
            }
        }
        std::free(buf);
    }
}

int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    printf("%6s  %-20s  %-5s  %-22s  %-22s\n",
           "PID", "Process", "Proto", "Local address", "Remote address");
    printf("--------------------------------------------------------------------------\n");

    PrintTcpTable();
    PrintUdpTable();

    WSACleanup();
    return 0;
}
