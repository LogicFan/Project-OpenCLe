#include "logger.hpp"

int main(int argc, char *argv[]) {
    std::cout << "-- BEGIN OF TEST ---" << std::endl;

    #ifdef NDEBUG
    std::cout << "There should print nothing." << std::endl;
    #endif
    
    logger << "This line should print with header if NDEBUG is not defined. ";
    logger_continue << "This line should continue to print is NDEBUG is not defined" << std::endl;
    
    std::cout << "--- END OF TEST ---" << std::endl;
    return 0;
}