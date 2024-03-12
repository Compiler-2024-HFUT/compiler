#ifndef TYPE__HPP
#define TYPE__HPP
#include <string>
#include <utility>
#include <vector>
#include "frontend/lex.hpp"
using std::string,std::vector;
namespace type {



#define TYPE_CONST (1<<0)
#define TYPE_STATIC (1<<1)
#define TYPE_U_INT (1<<3)
#define TYPE_INT (1<<2)
#define TYPE_FLOAT (1<<4)
// #define TYPE_STRUCT (1<<5)
//void only in func
#define TYPE_VOID (1<<7)

#define IS_CONST(i)  (TYPE_CONST&i)
#define IS_STATIC(i) (TYPE_STATIC&i)
#define IS_U_INT(i) (TYPE_U_INT&i)
#define IS_INT(i) (TYPE_INT&i)
#define IS_FLOAT(i) (TYPE_FLOAT&i)
// #define IS_STRUCT(i) (TYPE_STRUCT&i)
#define IS_VOID(i) (TYPE_VOID&i)


#define ADD_CONST(i)  (i=(TYPE_CONST|i))
#define ADD_STATIC(i) (i=(TYPE_STATIC|i))
#define ADD_U_INT(i) (i=(TYPE_U_INT|i))
#define ADD_INT(i) (i=(TYPE_INT|i))
#define ADD_FLOAT(i) (i=(TYPE_FLOAT|i))
// #define ADD_STRUCT(i) (i=(TYPE_STRUCT|i))
#define ADD_VOID(i) (i=(TYPE_VOID|i))


#define IS_NUM(i)  ((TYPE_FLOAT|TYPE_INT)&i)
typedef int type ;
struct StructType;

union Info{
    StructType * pointer;
    long size;
    Info(long );
};
struct ValType{
    // int i;
    // type t;
    type t;
    Info size;
    ValType();
};
struct StructType{
    string name;
    vector<std::pair<ValType, string>> members;
};

// struct Name  {
// 	string value;
// };

// struct Type  {
// };
struct Array  {
    long len  ;
    ValType elem;
};

// An Object describes a named language entity such as a package,
// constant, type, variable, function (incl. methods), or label.
// All objects implement the Object interface.
// 



// A Basic represents a basic type.
struct Basic  {
    // BasicKind kind;
    ValType info ;
    string name ;
};
struct Struct  {
    string name;
    std::vector<std::pair<ValType,string>> type_name;
};
struct Pointer  {
    ValType base; // element type
};





}
// enum ValType{
//     INT_VAL=1,
//     INT_CONST,
//     INT_POINT,
//     INT_POINT_CONST,
//     FLOAT_VAL,
//     FLOAT_CONST,
//     FLOAT_POINT,
//     FLOAT_POINT_CONST,
//     VOID_VAL,
// };
// struct type{
//     bool is_float=0;
//     bool is_const=0;
//     bool is_void=0;
//     bool un_useb=0;
// };
#endif