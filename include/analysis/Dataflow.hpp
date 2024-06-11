#ifndef DATAFLOW_HPP
#define DATAFLOW_HPP

#include "analysis/Info.hpp"

#include <map>
#include <set>

using std::map;
using std::set;

class Function;
class BasicBlock;
class Value;

using BB = BasicBlock;

/*
    包括如下数据流分析：
    1. 活跃变量分析
    2. 可达定义分析
 */

class LiveVar: public FunctionInfo {
private:
    map<BB*, set<Value*> > in;
    map<BB*, set<Value*> > out;
public:
    set<Value*> getLiveVarIn(BB* bb) {
        if(isInvalid()) 
            reAnalyse();
        return in[bb]; 
    }
    set<Value*> getLiveVarOut(BB* bb) { 
        if(isInvalid()) 
            reAnalyse();
        return out[bb]; 
    }

    virtual void analyse() override;
    virtual void reAnalyse() override;

    LiveVar(Function*func): FunctionInfo(func) {}
    ~LiveVar() {}
};

class ReachDef: public FunctionInfo {
private:
    map<BB*, set<Value*> > in;
    map<BB*, set<Value*> > out;
public:
    set<Value*> getReachDefIn(BB* bb) {
        if(isInvalid()) 
            reAnalyse();
        return in[bb]; 
    }
    set<Value*> getReachDefOut(BB* bb) { 
        if(isInvalid()) 
            reAnalyse();
        return out[bb]; 
    }

    virtual void analyse() override;
    virtual void reAnalyse() override;

    ReachDef(Function*func): FunctionInfo(func) {}
    ~ReachDef() {}
};

#endif