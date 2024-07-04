//该文件里写ASM的打印
#ifndef ASMPRINT_HPP
#define ASMPRINT_HPP

#include <string>

class AsmString{
    public:
        //单个双引号
        static const ::std::string dq;
        //空格
        static const ::std::string space;
        //换行
        static const ::std::string newline;
        //逗号
        static const ::std::string comma;
        //冒号
        static const ::std::string colon;

};

const ::std::string AsmString::dq = "\"";
const ::std::string AsmString::space = "    ";
const ::std::string AsmString::newline = "\n";
const ::std::string AsmString::comma = ", ";
const ::std::string AsmString::colon = ":";
#endif