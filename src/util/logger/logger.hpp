#include <iomanip>
#include <iostream>
#include <sstream>

#include "../core_def.hpp"

#ifndef NDEBUG
#define logger(msg)                                                            \
    std::cout << "[" << std::left << std::setw(20) << __FILE__ << ", "         \
              << std::setw(4) << __LINE__ << "] " << std::right << msg
#define logger_continue(msg) std::cout << msg
#else
#define logger(msg)
#define logger_continue(msg)
#endif