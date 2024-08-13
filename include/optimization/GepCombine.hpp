#include "analysis/Dominators.hpp"
#include "analysis/Info.hpp"
#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
#include "optimization/PassManager.hpp"
#include <vector>
//breakgep之后要运行vn 之后再运行
class GepCombine:public FunctionPass{
    std::vector<GetElementPtrInst*>work_set_;
    Dominators*dom;
    void init()override{
        dom=info_man_->getInfo<Dominators>();
    }
public:
    using FunctionPass::FunctionPass;
    bool  immOverRange(Function*func);
    bool adduseOneCombine(Function*func);
    bool  immOverRangeOnBB(BasicBlock*bb,    std::vector<std::pair<GetElementPtrInst*, int const>>gep_offset);
    Modify runOnFunc(Function*func)override;
};