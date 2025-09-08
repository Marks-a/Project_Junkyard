#ifndef CPP_TOOLBOX_HPP
#define CPP_TOOLBOX_HPP

#include <iostream>
#include <type_traits> // <- is_same / is_string
#include <string> // string
#include <utility>

// ========== Type Trait Helpers ==========
template<typename T>
constexpr bool is_string_v = std::is_same_v<std::decay_t<T>, std::string>;

template<typename T>
constexpr bool is_c_string_v = 
    std::is_same_v<std::decay_t<T>, const char*> || 
    std::is_same_v<std::decay_t<T>, char*>;

// ========== Debugging ==========
template<typename T>
void printType(const T& x) {
    if constexpr (std::is_integral_v<T>)
        std::cout << "Integral: " << x << '\n';
    else if constexpr (std::is_floating_point_v<T>)
        std::cout << "Float: " << x << '\n';
    else if constexpr (is_string_v<T> || is_c_string_v<T>)
        std::cout << "String: " << x << '\n';
    else
        std::cout << "Unknown Type\n";
}

// ========== Variadic Logger ==========
template<typename... Args>
void log(Args&&... args) {
    ((std::cout << std::forward<Args>(args) << ' '), ...) << '\n';
}

#endif // CPP_TOOLBOX_HPP
