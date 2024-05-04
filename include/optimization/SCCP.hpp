/*
    ref: llvm 1.0 : llvm_1.0.x/llvm/lib/Transforms/Scalar/SCCP.cpp
         稀疏条件常数传播（SCCP）, 用于SSA的常量传播 : 
         https://zhuanlan.zhihu.com/p/434113528
         && 
         https://karkare.github.io/cs738/lecturenotes/11CondConstPropSlides.pdf
 */

#ifndef CONST_PROP_HPP
#define CONST_PROP_HPP

#include "midend/Module.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Value.hpp"
#include "midend/Constant.hpp"
#include "PassManager.hpp"
#include <utility>
#include <vector>
#include <set>
#include <map>

// namespace transform ??

struct InstVal {
    enum {
        undef,          // top                      undef meet all = all
        constant,
        NaC             // bottom, not a const      nac meet all = nac
    } state;        // 当前指令值的状态，半格（SemiLattice）的三个顺序
    Constant *val;  // 当state = undef 或 Nac 时，此值无效 
    InstVal() : state(undef), val(nullptr) {}

    Constant *getConst() { return (state == constant) ? val : nullptr; }

    bool isUndef() { return (state == undef); }
    bool isConst() { return (state == constant); }
    bool isNaC() { return (state == NaC); }
    // markXX meaning this assign XX
    // if state change, return true
    bool markConst(Constant *constVal) {
        if (state != constant) {
            state = constant;
            val = constVal;
            return true;
        } else {
            // assert(val == constVal && "Marking constant with different value");
        }
        return false;
    }
    bool markNac() {
        if(state != NaC) {
            state = NaC;
            return true;
        }
        return false;
    }
};

// 完成mem2reg后可执行
// 执行后哪些analysis会无效，需要更新？
class SCCP : public FunctionPass {    
    struct Edge {
        std::pair<Value*, Value*> edge;
        enum {
            Flow,   // edge is CFG's edge, pair(block,block)
            SSA     // edge is a use-def chain(use_list_ in Value), pair(value,value)
        } type;

        static Edge makeFlowEdge(BasicBlock* from, BasicBlock* to) { return Edge{std::pair<Value*, Value*>(from, to), Flow}; }
        static Edge makeSSAEdge(Value* def, Value* use) { return Edge{std::pair<Value*, Value*>(def, use), SSA}; }
        bool isFlowEdge() { return type == Flow; }
        bool isSSAEdge()  { return type == SSA; }

        bool operator <(const Edge &other) const {
            return edge < other.edge;
        }
    };

    std::vector<Edge> worklist;
    std::map<Edge, int> execFlag;           // 不能找到或为0等价于false
    std::map<Value*, InstVal> LattValue;    // vlaue对应的半格值，找不到等价于undef

    InstVal &getInstVal(Value *v);
    int getExecFlag(Edge e);
    void addFlowEdge(BasicBlock *from);
    void addSSAEdge(Value *def);

    void visitInst(Instruction *i);
    void visitPhi(PhiInst *phi);

    // 实现单条指令(仅包含二元运算和比较运算)的常量折叠
    // 是否可以实现对Constant的运算符重载简化该函数实现？
    Constant *foldConst(Instruction *inst);

    bool runOnFunction(Function *f);
public:
    SCCP(Module *m) : FunctionPass(m) {}
    ~SCCP() {}
    
    void run() override;
};

#endif