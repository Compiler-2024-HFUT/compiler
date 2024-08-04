#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>
#include <cstdlib>

// #define ENABLE_LOGGER

// string str
// str += STRING(str1) + str2 + ...
#define STRING(msg)         ((std::string)(msg))
#define STRING_NUM(num)     (std::to_string(num))

#ifdef ENABLE_LOGGER
    // 颜色代码
    #define LOG_COLOR_RESET     STRING("\e[0m")
    #define LOG_COLOR_NORMAL    STRING("\e[7m")     // 反色 
    #define LOG_COLOR_YELLOW    STRING("\e[33m")    // 黄色
    #define LOG_COLOR_RED       STRING("\e[31m")    // 红色
    #define STRING_YELLOW(msg) (LOG_COLOR_YELLOW + msg + LOG_COLOR_RESET)
    #define STRING_RED(msg)    (LOG_COLOR_RED    + msg + LOG_COLOR_RESET)
    #define STRING_STRONG(msg) (LOG_COLOR_NORMAL + msg + LOG_COLOR_RESET)
    
    #define LOG_NORMAL(msg)         { std::cout << LOG_COLOR_NORMAL << "[LOG] " << msg << LOG_COLOR_RESET << std::endl; }

    // change to LOG_WARNING(cond, msg)
    #define LOG_WARNING(msg)        { std::cout << LOG_COLOR_YELLOW << "[WARNING] " << msg << LOG_COLOR_RESET << std::endl; }
    
    #define LOG_ERROR(msg, cond)    if(cond) {                                                                              \
                                        std::cerr << LOG_COLOR_RED << "[ERROR] " << msg << LOG_COLOR_RESET << std::endl;    \
                                        std::cerr << LOG_COLOR_RED << "position: " << __FILE__ << ":" << __LINE__ <<        \
                                                     ", in function: " << __FUNCTION__ << LOG_COLOR_RESET << std::endl;     \
                                        assert(0);                                                                          \
                                    }
#else
    // 颜色代码
    #define LOG_COLOR_RESET     STRING("")
    #define LOG_COLOR_NORMAL    STRING("")
    #define LOG_COLOR_YELLOW    STRING("")
    #define LOG_COLOR_RED       STRING("")
    #define STRING_YELLOW(msg)  STRING(msg)
    #define STRING_RED(msg)     STRING(msg)
    #define STRING_STRONG(msg)  STRING(msg)

    #define LOG_NORMAL(msg)     ;
    #define LOG_WARNING(msg)    ;
    #define LOG_ERROR(msg, cond) { assert( !(cond) && (msg) ); }
#endif

#endif