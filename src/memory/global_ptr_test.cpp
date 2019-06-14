#include <cassert>
#include "global_ptr.hpp"
#include <vector>

using namespace opencle;

int main() {
    global_ptr<int[]> g1;

    global_ptr<int[]> g2{10};
    
    std::unique_ptr<int[]> u3 = std::make_unique<int[]>(20);
    for(int i = 0; i < 20; ++i) {
        u3[i] = i * 2;
    }
    global_ptr<int[]> g3{std::move(u3), 20};
    for(int i = 0; i < 20; ++i) {
        std::cout << g3[i] << std::endl;
        assert(g3[i] == i * 2);
    }
    
    int *p4 = new int[20];
    for(int i = 0; i < 20; ++i) {
        p4[i] = i * 3;
    }
    global_ptr<int[]> g4{p4, 20};
    delete p4;
    for(int i = 0; i < 20; ++i) {
        std::cout << g4[i] << std::endl;
        assert(g4[i] == i * 3);
    }

    std::vector<int> v5 = {1, 2, 3, 4, 5};
    global_ptr<int[]> g5{v5};
    for(int i = 0; i < 5; ++i) {
        std::cout << g5[i] << std::endl;
        assert(g5[i] == i + 1);
    }

    global_ptr<int[]> g6{std::move(g5)};
    for(int i = 0; i < 5; ++i) {
        std::cout << g6[i] << std::endl;
        assert(g6[i] == i + 1);
    }

    global_ptr<int[]> g7{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    for(int i = 0; i < 10; ++i) {
        std::cout << g7[i] << std::endl;
        assert(g7[i] == i);
    }
}