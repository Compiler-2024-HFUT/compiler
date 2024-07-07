#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>
#include <cstdlib>

#define ENABLE_LOGGER

// string str
// str += STRING(str1) + str2 + ...
#define STRING(msg)         ((std::string)(msg))
#define STRING_NUM(num)     (std::to_string(num))

// 颜色代码
#define LOG_COLOR_RESET     STRING("\e[0m")
#define LOG_COLOR_NORMAL    STRING("\e[7m")     // 反色 
#define LOG_COLOR_YELLOW    STRING("\e[33m")    // 黄色
#define LOG_COLOR_RED       STRING("\e[31m")    // 红色

#ifdef ENABLE_LOGGER
    #define STRING_YELLOW(msg) (LOG_COLOR_YELLOW + msg + LOG_COLOR_RESET)
    #define STRING_RED(msg)    (LOG_COLOR_RED    + msg + LOG_COLOR_RESET)
    #define STRING_STRONG(msg) (LOG_COLOR_NORMAL + msg + LOG_COLOR_RESET)
    
    #define LOG_NORMAL(msg) std::cout << LOG_COLOR_NORMAL << "[LOG] " << msg << LOG_COLOR_RESET << std::endl;
    #define LOG_WARNING(msg) std::cout << LOG_COLOR_YELLOW << "[WARNING] " << msg << LOG_COLOR_RESET << std::endl;
    #define LOG_ERROR(msg, code) std::cerr << LOG_COLOR_RED << "[ERROR] " << msg << LOG_COLOR_RESET << std::endl; exit(code);
#else
    #define LOG_NORMAL(msg) 
    #define LOG_WARNING(msg)    
    #define LOG_ERROR(msg, code)
#endif



// frontendReport

// middleendReport

// backendReport

#endif