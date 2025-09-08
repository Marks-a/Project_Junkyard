#include <iostream>
#include <cstdio>
#include <vector>
#include <type_traits> 

template<
    typename T,
    typename = std::enable_if_t<std::is_integral<T>::value>>
    class IntList
{
    std::vector<T> data;
public:
    void push(T x) { data.push_back(x); }
    T get(size_t i) const { return data[i]; }
};

void log() { std::cout << '\n'; };
template<typename T, typename... Rest>
 
void log(T &&first, Rest &&...rest)
{
    std::cout << std::forward<T>(first);
    if constexpr (sizeof...(rest) > 0)
        std::cout << ' ';
    log(std::forward<Rest>(rest)...);
}

int main(int argc, char *argv[])
{
    IntList<int> list;
    list.push(42);
    printf("%d\n", list.get(0));
    return 0;
}