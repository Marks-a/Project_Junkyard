/*
    DONT WORK
    ProcKill <PID>
    ProcKill -n <process.exe> 
    gcc ProcKill.c -o ProcKill.exe -lpsapi
*/

#define _WIN32_WINNT 0x0600       
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <tchar.h>
#include <ctype.h>

void kill_by_pid(DWORD pid)
{
    HANDLE h = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!h) {
        printf("  PID %lu – unable to open (err=%lu)\n", pid,
               GetLastError());
        return;
    }

    TCHAR img[MAX_PATH] = _T("<unknown>");
    GetModuleBaseName(h, NULL, img, MAX_PATH);

    if (TerminateProcess(h, 1)) {
        printf("PID %lu (%ls)  terminated\n", pid, img);
    } else {
        printf("PID %lu (%ls)  terminate failed (err=%lu)\n", pid, img, GetLastError());
    }
    CloseHandle(h);
}

void to_lower(char *s)
{
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

void kill_by_name(const char *name)
{
    char target[MAX_PATH];
    strncpy(target, name, sizeof(target));
    target[sizeof(target)-1] = '\0';
    to_lower(target);
    DWORD pids[1024], needed;
    if (!EnumProcesses(pids, sizeof(pids), &needed)) {
        printf("Enumuration processes failed (err=%lu)\n", GetLastError());
        return;
    }
    int total = needed / sizeof(DWORD), killed = 0;

    for (int i = 0; i < total; ++i) {
        DWORD pid = pids[i];
        HANDLE h = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION |
                               PROCESS_VM_READ, FALSE, pid);
        if (!h) continue;

        char imgA[MAX_PATH] = "<unknown>";
        HMODULE mod; DWORD cb;
        if (EnumProcessModules(h, &mod, sizeof(mod), &cb)) {
            GetModuleBaseNameA(h, mod, imgA, sizeof(imgA));
        }
        char imgLow[MAX_PATH];
        strncpy(imgLow, imgA, sizeof(imgLow));
        imgLow[sizeof(imgLow)-1] = '\0';
        to_lower(imgLow);

        if (strcmp(imgLow, target) == 0) {
            if (TerminateProcess(h, 1)) {
                printf("PID %lu (%s) terminated\n", pid, imgA);
                ++killed;
            } else {
                printf("PID %lu (%s) terminate failed (err=%lu)\n",
                       pid, imgA, GetLastError());
            }
        }
        CloseHandle(h);
    }
    if (!killed) printf("No processes named \"%s\" found.\n", name);
}

int main(int argc, char *argv[])
{
    if (argc == 2) {                      
        DWORD pid = strtoul(argv[1], NULL, 0);
        if (pid == 0) {
            puts("Invalid PID.");
            return 1;
        }
        kill_by_pid(pid);
    } else if (argc == 3 && strcmp(argv[1], "-n") == 0) { 
        kill_by_name(argv[2]);
    } else {
        printf("\n"
               "%s <PID> terminate by PID\n"
               "%s -n <process.exe> terminate by name\n",
               argv[0], argv[0]);
        return 1;
    }
    return 0;
}
