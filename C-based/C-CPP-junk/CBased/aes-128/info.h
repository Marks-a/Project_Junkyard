#ifndef info_h
#define info_h

#include <stdio.h>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/utsname.h>

#endif

static inline void print_system_info()
{
#ifdef _WIN32
    printf("Operating System: Windows\n");
#elif __linux__
    printf("Operating System: Linux\n");
#elif __APPLE__
    printf("Operating System: macOS\n");
#else
    printf("Operating System: Unknown\n");
#endif

#ifdef __x86_64__
    printf("Architecture: x86_64\n");
#elif __i386__
    printf("Architecture: x86 (32-bit)\n");
#elif __aarch64__
    printf("Architecture: ARM64\n");
#elif __arm__
    printf("Architecture: ARM\n");
#else
    printf("Architecture: Unknown\n");
#endif

#if defined(__linux__) || defined(__APPLE__)
    struct utsname sysinfo;
    if (uname(&sysinfo) == 0)
    {
        printf("System Name: %s\n", sysinfo.sysname);
        printf("Node Name: %s\n", sysinfo.nodename);
        printf("Release: %s\n", sysinfo.release);
        printf("Version: %s\n", sysinfo.version);
        printf("Machine: %s\n", sysinfo.machine);
    }
    else
    {
        perror("uname");
    }
#endif
}

static inline void print_menu()
{
    printf("\n--- AES-128 Cipher Menu ---\n");
    printf("1. Encrypt a file\n");
    printf("2. Decrypt a file\n");
    printf("3. Exit\n");
    printf("Enter your choice: ");
}
#endif
