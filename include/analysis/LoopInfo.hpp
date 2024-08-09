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
#include <algorithm>
using std::find;
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

    Instruction *transToInst(BB *bb) {
        umap<opType, CmpOp> opMap = {
            {opType::gt, CmpOp::GT},
            {opType::ge, CmpOp::GE},
            {opType::lt, CmpOp::LT},
            {opType::le, CmpOp::LE},
            {opType::eq, CmpOp::EQ},
            {opType::ne, CmpOp::NE},
        };
        return CmpInst::createCmp(opMap[op], lhs, rhs, bb);
    }
    static LoopCond *createLoopCond(Instruction *inst) {
        if(!inst->isCmp())  // fcmp?
            return nullptr;
        
        opType op;
        CmpInst *cmp = dynamic_cast<CmpInst*>(inst);
        umap<CmpOp, opType> opMap = {
            {CmpOp::GT, opType::gt},
            {CmpOp::GE, opType::ge},
            {CmpOp::LT, opType::lt},
            {CmpOp::LE, opType::le},
            {CmpOp::EQ, opType::eq},
            {CmpOp::NE, opType::ne},
        };

        op = opMap[cmp->getCmpOp()];
        return new LoopCond{cmp->getOperand(0), op, cmp->getOperand(1)};
    }
    ~LoopCond() { delete this; }
};

class SCEV;
struct SCEVExpr;
struct LoopTrip {
    Value *start;  // [start, end)
    Value *end;
    int iter;   // 每次迭代量
    int step;   // 总步数，0 when loop never run, -1 when step can't get, -2 when dead loop, -3 dynamic loop
    static LoopTrip createEmptyTrip(int n) { return {nullptr, nullptr, 0, n}; } 
    string print() {
        if(step == 0) {
            return "{ loop never run }\n";
        } else if(step == -1) {
            return "{ can't get loop trip }\n";
        } else if (step == -2) {
            return "{ dead loop }\n";
        } else {
            return "{ range: [" + STRING(start->getName()) + "," + STRING(end->getName()) + ")/], iter: "
            +  STRING_NUM(iter) + ", total step: " + STRING_NUM(step) + " }\n";
        }
    }
};

class Loop {
    BB *preheader = nullptr;
    BB *header = nullptr;  

    vector<BB*> latchs = {};            // 有一条边连接到header, latch->header形成回边backedge
    BB *latch = nullptr;                // 仅只有一个latch时有效，多于1个为空

    vector<BB*> exits = {};             // 执行完循环或break后到达的基本块   
    // BB *exit = nullptr;              // 仅只有一个exit时有效，多于1个为空

    uset<BB*> blocks = {};              // 组成natural loop的集合
                     
    Loop *outer = nullptr;              // 外层循环
    vector<Loop*> inners = {};          // 内循环
    int depth = 1;                      // 循环深度, 最外层深度为1

    PhiInst *indVar = nullptr;

    bool simple = false;                // 是否已经简化
    vector<LoopCond*> conditions;       // cond1 && cond2... 暂不考虑 或 的情况，出现则conditions为空
public:
    Loop(BB *tail, BB *head): latchs({tail}), header(head) {}

    PhiInst *getIndVar() { return indVar; }
    Function *getFunction() { return header->getParent(); }
    BB* getHeader() { return header; }
    BB* getPreheader() { return preheader; }
    BB* getSingleLatch() { return latch; }
    int getDepth() { return depth; }
    Loop *getOuter() { return outer; }

    void setIndVar(PhiInst *iv) { indVar = iv; }
    void setPreheader(BB *ph) { 
        preheader = ph;
        if(outer)
            outer->addBlock(ph);
    }
    void setDepth(int d) { depth = d; }
    void setOuter(Loop *l) { outer = l; }
    void setSingleLatch(BB *sl) { 
        latch = sl; 
        if(!latch) { blocks.erase(latch); } 
        addBlock(sl);
    }

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
    void replaceCond(LoopCond *newCond);
    void addWeakCond(LoopCond *weakCond);

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
    void addLatch(BB *bb) { latchs.push_back(bb); addBlock(bb); }
    void addBlock(BB *bb) { 
        blocks.insert(bb);
        if(outer)
            outer->addBlock(bb);
    }
    void addBlocks(vector<BB*> bbs) {
        for(BB *bb : bbs)
            blocks.insert(bb);
        if(outer)
            outer->addBlocks(bbs);
    }
    void addInner(Loop *l) { inners.push_back(l); }

    void findExits();

    bool contain(BB *bb) { return blocks.count(bb) > 0; }

    // 复制除header外的blocks，参数返回进入loopBody的entry和跳出循环的exiting(exit的preBB)，原BB和新BB的指令映射
    void copyBody(BB* &entry, BB* &singleLatch, vector<BB*> &exiting, map<BB*, BB*> &BBMap, map<Instruction*, Instruction*> &instMap);
    // 复制简化后的Loop
    Loop *copyLoop();

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
        for(auto bb : exits) {
            loop += '\t' + bb->getName() + '\n';
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
    void findExits(Function *func_);
    void combineLoops(Function *func_);         // 对于存在continue的循环，存在多条backedge回到header，该方法主要是将具有相同header的Loop合并
    void producedNestLoops(Function *func_);    // 处理嵌套循环，计算loop的inner
    void DFS_CFG( BB* rootBB, int level ); 
    void DFS_CFG( BB* tail, BB* head, Loop *loop, Function *func_);
public:
    LoopInfo(Module *m, InfoManager *im): FunctionInfo(m, im) { this->mod.modify_bb=true;}
    virtual ~LoopInfo() { }

    virtual void analyseOnFunc(Function *func) override;

    virtual string print() override;

    vector<Loop*> getLoops(Function* func) { 
        if(isInvalid()) analyse();
        return loops[func]; 
    }

    void removeLoop(Loop *loop) {
        std::function<void(Loop*, Loop*)> removeL = [&](Loop *outer, Loop *toDel) {
            for(auto iterL = outer->getInners().begin(); iterL != outer->getInners().end(); iterL++) {
                if(*iterL == toDel) {
                    outer->getInners().erase(iterL);
                    return;
                }
            }
            
            for(Loop *inner : outer->getInners()) {
                removeL(inner, toDel);
            }
        };
        
        Function *func = loop->getFunction();
        auto pos = find(loops[func].begin(), loops[func].end(), loop);
        if(pos!= loops[func].end()) {
            loops[func].erase(pos);
            return;
        }

        // 删除的是子循环
        for(Loop *l : loops[func])
            removeL(loop->getOuter(), loop);
    }
};

#endif