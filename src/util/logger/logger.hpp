#include <iomanip>
#include <iostream>
#include <sstream>
#include "../core_def.hpp"

#ifndef NDEBUG
#define logger                                                                 \
    std::cout << "[" << std::left << std::setw(20) << __FILE__ << ", "         \
              << std::setw(4) << __LINE__ << "] " << std::right
#else 
std::stringstream ss;
#define logger ss.clear(); ss
#endif