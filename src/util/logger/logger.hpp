#include <iomanip>
#include <iostream>
#include <sstream>

#include "../core_def.hpp"

#ifndef NDEBUG
#define logger                                                                 \
    std::cout << "[" << std::left << std::setw(20) << __FILE__ << ", "         \
              << std::setw(4) << __LINE__ << "] " << std::right
#define logger_continue std::cout
#else
static std::stringstream _logger_ndebug_stringstream_;

#define logger _logger_ndebug_stringstream_.clear(); _logger_ndebug_stringstream_
#define logger_continue _logger_ndebug_stringstream_.clear(); _logger_ndebug_stringstream_
#endif