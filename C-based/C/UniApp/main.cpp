#include "Utils/utils.hpp"

int main()
{
    if (IsRunningAsAdmin())
    {
        std::cout << "[+] Running as Administrator" << std::endl;
    }
    else
    {
        std::cout << "[-] Not running as Administrator" << std::endl;
    }

   show_menu();
  
    return 0;
}