#include "utils.hpp"

struct sCommand
{
    std::string name;
    std::function<void()> action;
};


std::vector<sCommand> menu = {
    {"Show System Info", print_system_info},
  //  {"Detailed Com function", Com},
    {"Proxy", proxy_settings_menu},
    {"Exit",  []() { exit(0); }}
};

void show_menu() {
    while (true) {
        std::cout << "\n=== Main Menu ===\n";
        for (size_t i = 0; i < menu.size(); ++i) {
            std::cout << "[" << i + 1 << "] " << menu[i].name << "\n";
        }

        std::cout << "> ";
        int choice;
        std::cin >> choice;

        if (choice > 0 && choice <= static_cast<int>(menu.size())) {
            std::cout << "\n";
            menu[choice - 1].action();
        } else {
            std::cout << "Invalid choice. Try again.\n";
        }
    }
}




