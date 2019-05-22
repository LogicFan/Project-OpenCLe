#include <iomanip>
#include <iostream>
#include <sstream>

#include "../core_def.hpp"

#ifndef NDEBUG
#define logger                                                                 \
    std::cout << "[" << std::left << std::setw(20) << __FILE__ << ", "         \
              << std::setw(4) << __LINE__ << "] " << std::right
#else
static std::stringstream _logger_ndebug_stringstream_;
inline std::stringstream &
_func_logger_ndebug_stringstream_(std::stringstream &ss) {
    ss.clear();
    return ss;
}
#define logger _func_logger_ndebug_stringstream_(_logger_ndebug_stringstream_)
#endif