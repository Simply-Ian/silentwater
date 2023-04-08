#include "Logger.h"
#include <chrono>
#include <ctime>


void Logger::log(std::string message, levels level){
    std::string level_str;
    switch (level){
        case levels::INFO: 
            level_str = "INFO";
            break;
        case levels::WARNING: 
            level_str = "WARNING";
            break;
        case levels::CRITICAL: 
            level_str = "CRITICAL";
            break;
    }
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);
    std::string header = "[" + std::to_string(1900 + now->tm_year) + "." 
                                + std::to_string(now->tm_mon) + "." 
                                + std::to_string(now->tm_mday) + " " 
                                + std::to_string(now->tm_hour) + ":" + std::to_string(now->tm_min) + ":" 
                                + std::to_string(now->tm_sec) + "] ";
    header += level_str + ":\n";
    FILE << header.c_str();
    FILE << message.c_str() << "\n \n";
}