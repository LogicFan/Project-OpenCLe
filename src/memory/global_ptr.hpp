#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

namespace opencle {
template <typename T, typename X = void> class global_ptr;

template <typename T> class global_ptr<T[], std::enable_if_t<std::is_pod_v<T>>> {
  public:
    global_ptr() { std::cout << std::is_pod_v<T> << std::endl; }
};
} // namespace opencle

