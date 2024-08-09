#include "frontend/parser.hpp"
#include "midend/IRGen.hpp"
#include "optimization/PassManager.hpp"
#include "optimization/PassManagerBuilder.hpp"
#include "utils/Logger.hpp"

#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include <cstdlib>

using namespace std;

int main(int argc , char**argv){
    ofstream ofs1 = ofstream("/home/howldark/compiler/cfg1.dot");
    ofstream ofs2 = ofstream("/home/howldark/compiler/cfg2.dot");
    ofstream ofc1  = ofstream("/home/howldark/compiler/code1.ll");
    ofstream ofc2  = ofstream("/home/howldark/compiler/code2.ll");

    if(argc<2){
        std::cerr<<("expect argv")<<endl;
        exit(-1);
    }
    Parser *p=new Parser(argv[1]);
    p->parserComp();
    IRgen::IRGen irgen;
    p->comp->accept(irgen);
    auto m=irgen.getModule();
 
    PassManager *pm = new PassManager(m);
    buildTestPassManager(pm);
    pm->addPass<Mem2Reg>();
    pm->addPass<LoopSimplified>();
    
    // pm->run();

    // cout << m->print();
    // cout << pm->getInfo<LoopInfo>()->print();
    // cout << pm->getInfo<SCEV>()->print();
    // ofc1 << m->print();
    // ofs1 << irgen.getModule()->printGra();

    // cout << pm->getInfo<LoopInvariant>()->print();
    
    // cout << pm->getInfo<LiveVar>()->print();
    // pm->addPass<LoopSimplified>();
    
    // pm->addPass<LoopStrengthReduction>();
    // pm->addPass<LICM>();
    // pm->addPass<LoopUnroll>();
    // pm->run();
    cout << m->print();
    // cout << pm->getInfo<LoopInfo>()->print();
    // cout << pm->getInfo<LoopInvariant>()->print();
    // ofc2 << m->print();
    // ofs2 << irgen.getModule()->printGra();
    
    // pm->getInfo<Dominators>()->printDomSet();
    // pm->getInfo<Dominators>()->printDomFront();
    // pm->getInfo<Dominators>()->printDomTree();

    
    // cout << pm->getInfo<SCEV>()->print();
    // cout << pm->getInfo<LiveVar>()->print();

    ofs1.close();
    ofs2.close();
    ofc1.close();
    ofc2.close();
    delete (p);

}