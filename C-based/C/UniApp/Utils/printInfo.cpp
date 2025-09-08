#include "utils.hpp"


std::string get_motherboard_info();
std::string get_cpu_info();
std::string get_bios_info();
std::string get_memory_info();
std::string get_memory_details();
std::string get_gpu_info();
std::string get_disk_info();
std::string get_disk_logical_info();
std::string get_network_info();
std::string get_os_info();
std::string get_cs_product_info();
std::string exec_cmd(const char *cmd);


void Com()
{
    std::vector<std::pair<std::string, std::function<void()>>> list = {
        // {"System Info", []()
        //  { print_system_info(); }},
        {"Motherboard Info", get_motherboard_info},
        {"CS Product Info", get_cs_product_info},
        {"CPU Info", get_cpu_info},
        {"BIOS Info", get_bios_info},
        {"Memory Info", get_memory_info},
        {"Memory Details", get_memory_details},
        {"GPU Info", get_gpu_info},
        {"Disk Info", get_disk_info},
        {"Disk Logical Info", get_disk_logical_info},
        {"Network Info", get_network_info},
        {"OS Info", get_os_info}

    };

    /*
    Thread related code
    This will create a thread for each item in the list and execute the function in that thread
    */
    std::vector<HANDLE> handles;
   
    
    WaitForMultipleObjects((DWORD)handles.size(), handles.data(), TRUE, INFINITE);
    for (HANDLE h : handles) CloseHandle(h);
    

}

void print_system_info()
{
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);

    std::cout << "[+] CPU Cores: " << siSysInfo.dwNumberOfProcessors << std::endl;

    OSVERSIONINFOEX osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx((OSVERSIONINFO *)&osvi);

    std::cout << "[+] OS Version: " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion << std::endl;
}

std::string get_motherboard_info()
{
    return exec_cmd("wmic baseboard get Manufacturer,Product,SerialNumber,Version");
}

std::string get_cs_product_info() {
    return exec_cmd("wmic csproduct get Name,IdentifyingNumber,UUID");
}

std::string get_cpu_info()
{
   return exec_cmd("wmic cpu get Name,NumberOfCores,NumberOfLogicalProcessors,MaxClockSpeed,Manufacturer");
   
}

std::string get_bios_info()
{
    return exec_cmd("wmic bios get Manufacturer,SMBIOSBIOSVersion,Version");
}

std::string get_memory_info()
{
    return exec_cmd("wmic memorychip get Capacity,Manufacturer,Speed,PartNumber");
}
std::string get_memory_details()
{
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);
    GlobalMemoryStatusEx(&memInfo);
    
    std::string res = "Total Physical RAM: " + std::to_string(memInfo.ullTotalPhys / (1024 * 1024 * 1024)) + " GB\n" +
                        "Available RAM: " + std::to_string(memInfo.ullAvailPhys / (1024 * 1024 * 1024)) + " GB\n" +
                        "Virtual Memory: " + std::to_string(memInfo.ullTotalVirtual / (1024 * 1024 * 1024)) + " GB\n";


    std::cout << "\n get_memory_details size of res: " << res.size() << ", sizeof of res: " << sizeof(res) << std::endl;
    return res;
}

std::string get_gpu_info()
{
    return exec_cmd("wmic path win32_VideoController get Name,DriverVersion,AdapterRAM");
}

std::string get_disk_info()
{
    return exec_cmd("wmic diskdrive get Model,InterfaceType,SerialNumber,Size");
}
std::string get_disk_logical_info() {
    return exec_cmd("wmic logicaldisk get Name,FileSystem,FreeSpace,Size,VolumeName");
}

std::string get_network_info()
{
    return exec_cmd("ipconfig /all");
}

std::string get_os_info()
{
    return exec_cmd("systeminfo | findstr /B /C:\"OS Name\" /C:\"OS Version\" /C:\"System Boot Time\"");
}
   
std::string exec_cmd(const char *cmd)
{
    std::array<char, CharBufferSize> buffer;
    std::string result;

    // Open pipe for reading
    FILE *pipe = popen(cmd, "r");
    if (!pipe)
        return "Cannon open pipe";

    // Read until EOF
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}