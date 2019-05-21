#include <iomanip>
#include <iostream>
#define logger                                                                 \
    std::cout << "[" << std::left << std::setw(20) << __FILE__ << ", "         \
              << std::setw(4) << __LINE__ << "] " << std::right
