#include "optimization/PassManager.hpp"
#include <functional>
void usage();
class Compiler{
    enum OPT:int{
        NONE=0,
        FAST=1,
        FASTER=2,
    } opt_level=OPT::NONE;
    bool is_out_asm=false;
    bool is_out_llvm=false;
    char const * out_name=nullptr;
    char const *in_name=nullptr;
    std::function<void(PassManager&pm)> const lir;
    void buildDefault(PassManager &pm);
    void buildOpt(PassManager &pm);
public:
    Compiler(int argc ,char**argv);
    int run();
};