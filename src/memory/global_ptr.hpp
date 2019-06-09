#include <type_traits>
#include <typeinfo>
#include <utility>
#include <iostream>
#include <vector>

namespace opencle {
template <typename T, typename X = std::enable_if<std::is_pod_v<T>>> class global_ptr {
    public:
    global_ptr() { std::cout << typeid(T).name() << std::endl; }
};
} // namespace opencle

int main() { opencle::global_ptr<std::vector<int>> a; }
