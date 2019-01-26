#include "Device/Platform.hpp"
#include "Device/Device.hpp"

#include <iostream>

using namespace std;

int main() {
    // test Platform.hpp/cpp
    auto vec = opencle::getPlatformList();
    for(auto &ele : vec) {
        ele.getDeviceList();
    }
    int x;
    cin >> x; 
}