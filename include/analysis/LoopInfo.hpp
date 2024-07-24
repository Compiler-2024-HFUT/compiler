#ifndef LOOPINFO_HPP
#define LOOPINFO_HPP

#include "midend/Module.hpp"
#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"
#include "analysis/Info.hpp"
#include "analysis/InfoManager.hpp"
#include "utils/Logger.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::map;
using std::pair;
template<typename key, typename value>
using umap = std::unordered_map<key, value>;
template<typename value>
using uset = std::unordered_set<value>;

using BB = BasicBlock;

// lhs op rhs
struct LoopCond {
    Value *lhs;
    enum opType {
        gt = -2, ge = -1, lt = 1, le = 2, eq, ne
    } op;
    Value *rhs;
    static LoopCond *createLoopCond(Instruction *inst) {
        if(!inst->isCmp())
            LOG_ERROR("loopcond is based on cmpinst!", 1)
        
        opType op;
        CmpInst *cmp = dynamic_cast<CmpInst*>(inst);
        switch (cmp->getCmpOp()) {
            case  CmpOp::EQ:
                op = opType::eq;
                break;
            case  CmpOp::NE:
                op = opType::ne;
                break;
            case  CmpOp::GT:
                op = opType::gt;
                break;
            case  CmpOp::GE:
                op = opType::ge;
                break;
            case  CmpOp::LT:
                op = opType::lt;
                break;
            case  CmpOp::LE:
                op = opType::le;
                break;
            default:
                assert(0);
                break;
        }

        return new LoopCond{cmp->getOperand(0), op, cmp->getOperand(1)};
    }
    ~LoopCond() { delete this; }
};

class SCEV;
struct SCEVExpr;
struct LoopTrip {
    int start;  // [start, end)
    int end;
    int iter;   
    int step;   // 0 when loop never run, -1 when step can't get, -2 when dead loop
    static LoopTrip createEmptyTrip(int n) { return {0, 0, 0, n}; } 
};

class Loop {
    BB *preheader = nullptr;
    BB *header;  

    vector<BB*> latchs;                 // 有一条边连接到header, latch->header形成回边backedge
    BB *latch = nullptr;                // 仅只有一个latch时有效，多于1个为空

    vector<BB*> exits;                  // 执行完循环或break后到达的基本块   
    BB *exit = nullptr;                 // 仅只有一个exit时有效，多于1个为空

    uset<BB*> blocks;                   // 组成natural loop的集合
                     
    Loop *outer;                        // 外层循环
    vector<Loop*> inners;               // 内循环
    int depth = 1;                      // 循环深度, 最外层深度为1

    bool simple = false;                // 是否已经简化
    vector<LoopCond*> conditions;       // cond1 && cond2... 暂不考虑 或 的情况，出现则conditions为空
public:
    Loop(BB *tail, BB *head): latchs({tail}), header(head) {}
    
    Function *getFunction() { return header->getParent(); }
    BB* getHeader() { return header; }
    BB* getPreheader() { return preheader; }
    BB* getSingleLatch() { return latch; }
    BB* getSingleExit() { return exit; }
    int getDepth() { return depth; }
    Loop *getOuter() { return outer; }

    void setPreheader(BB *ph) { preheader = ph; }
    void setDepth(int d) { depth = d; }
    void setOuter(Loop *l) { outer = l; }
    void setSingleLatch(BB *sl) { 
        latch = sl; 
        if(!latch) { blocks.erase(latch); } 
        blocks.insert(sl);
    }
    void setSingleExit(BB *se) { exit = se; }

    // 检查除header外，其它因短路求值而产生的表示条件判断的BB，匹配如下格式：
    // %op = icmp ...   ; 不考虑fcmp
    // br i1 %op ...
    static bool isCondBlock(BB *bb) {
        return ( bb->getNumOfInstrs()==2 && 
                 bb->getInstructions().front()->isCmp() &&
                 bb->getInstructions().back()->isBr() );
    }

    bool isSimplifiedForm() { return simple; }
    void setSimplified() { simple = true; }

    vector<LoopCond*> const &getConds() { return conditions; }
    bool computeConds();
    void setCondsAndUpdateBlocks(vector<LoopCond*> conds);

    LoopTrip computeTrip(SCEV *scev);

    // 从func中删除与该loop相关的所有基本块
    void removeLoop();  

    // 自身及子循环深度+1
    void updateDepth() {
        depth += 1;
        for(Loop *l : inners) {
            l->updateDepth();
        }
    }

    vector<BB*> &getExits()   { return exits; }
    vector<BB*> &getLatchs()  { return latchs; }
    uset<BB*> &getBlocks()    { return blocks; }
    vector<Loop*> &getInners()  { return inners; }
 
    void addExit(BB *bb) { exits.push_back(bb); }
    void addLatch(BB *bb) { latchs.push_back(bb); blocks.insert(bb); }
    void addBlock(BB *bb) { blocks.insert(bb); }
    void addInner(Loop *l) { inners.push_back(l); }

    bool contain(BB *bb) { return blocks.count(bb) > 0; }

    string print() {
        string loop = "";
        if(preheader != nullptr) 
            loop += STRING_YELLOW("preheader: ") + preheader->getName() + '\n';
        loop += STRING_YELLOW("header: ") + header->getName() + '\n';

        loop += STRING_YELLOW("latchs") + ":\n";
        if(latch == nullptr) {
            for(auto bb : latchs) {
                loop += '\t' + bb->getName() + '\n';
            }
        } else {
            loop += '\t' + latch->getName() + '\n';
        }
        
        loop += STRING_YELLOW("blocks") + ":\n";
        for(auto bb : blocks) {
            loop += '\t' + bb->getName() + '\n';
        }
        
        loop += STRING_YELLOW("exits") + ":\n";
        if(exit == nullptr) {
            for(auto bb : exits) {
                loop += '\t' + bb->getName() + '\n';
            }
        } else {
            loop += '\t' + exit->getName() + '\n';
        }

        if(inners.size() != 0) {
            loop += STRING_YELLOW("inners") + ":\n";
            for(Loop *l : inners) {
                loop += STRING("outer's header: ") + STRING_RED(header->getName()) + STRING(" inner's depth: ") + STRING_NUM(l->getDepth());
                loop += "\n" + l->print() + "\n";
            }
        }
        
        return loop;
    }
};

class LoopInfo: public FunctionInfo {
    umap<Function*, vector<Loop*> > loops;
    umap<BB*, int> BBlevel;    
    vector< pair<BB*, BB*> > retreatEdges;       // 由低层次指向高层次的边
    vector< pair<BB*, BB*> > backEdges;

    void findRetreatEdges(Function *func_);   
    void findBackEdges();
    void findLoops(Function *func_);
    void findExit(Function *func_);
    void combineLoops(Function *func_);         // 对于存在continue的循环，存在多条backedge回到header，该方法主要是将具有相同header的Loop合并
    void producedNestLoops(Function *func_);    // 处理嵌套循环，计算loop的inner
    void DFS_CFG( BB* rootBB, int level ); 
    void DFS_CFG( BB* tail, BB* head, Loop *loop, Function *func_);
public:
    LoopInfo(Module *m, InfoManager *im): FunctionInfo(m, im) { }
    virtual ~LoopInfo() { }

    virtual void analyseOnFunc(Function *func) override;

    virtual string print() override;

    vector<Loop*> getLoops(Function* func) { 
        if(isInvalid()) analyse();
        return loops[func]; 
    }

};

#endif