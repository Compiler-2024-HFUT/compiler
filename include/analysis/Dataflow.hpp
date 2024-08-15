#ifndef DATAFLOW_HPP
#define DATAFLOW_HPP

#include "analysis/Info.hpp"
#include "utils/Logger.hpp"

#include <map>
#include <set>
#include <utility>
using std::map;
using std::set;
using std::pair;

class Module;
class Function;
class BasicBlock;
class Value;

using BB = BasicBlock;

/*
    包括如下数据流分析：
    1. 活跃变量分析
    参考资料：SSA-based Compiler Design P110 的数据流方程
             编译器设计（第2版）P328

    2. 可达定义分析
 */

class LiveVar: public FunctionInfo {
private:
    // 将一个liveVar集合分成两个集合，一个存放int，一个float
    using IntSet = set<Value*>;
    using FloatSet = set<Value*>;
    using LiveSets = pair<IntSet, FloatSet>;    

    map<BB*, LiveSets> liveIn;
    map<BB*, LiveSets> liveOut;

    map<BB*, set<Value*> > Defs;
    map<BB*, set<Value*> > UEUses;    // Upward Exposed Uses, var在BB中被使用，并且var未在BB中定义
    map<BB*, set<Value*> > PhiDefs;
    map<BB*, set<Value*> > PhiUses;

    void initUseAndDef(Function *func_);
    void findFixedPoint(Function *func_);
public:
    LiveSets const &getLiveVarIn(BB* bb) { return liveIn[bb]; }
    LiveSets const &getLiveVarOut(BB* bb) { return liveOut[bb]; }

    IntSet const &getIntLiveVarIn(BB* bb) { return liveIn[bb].first; }
    IntSet const &getIntLiveVarOut(BB* bb) { return liveOut[bb].first; }

    FloatSet const &getFloatLiveVarIn(BB* bb) { return liveIn[bb].second; }
    FloatSet const &getFloatLiveVarOut(BB* bb) { return liveOut[bb].second; }

    // bool isLiveInProgramPoint(PP pp, Value var)...
    
    virtual void analyseOnFunc(Function *func) override;

    virtual string print() override;

    LiveVar(Module *m, InfoManager *im): FunctionInfo(m, im) {}
    ~LiveVar() {}
};

#endif