#include "frontend/parser.hpp"
#include "midend/IRGen.hpp"
#include "optimization/PassManager.hpp"
#include "optimization/PassManagerBuilder.hpp"
#include "utils/Logger.hpp"

#include <iostream>
#include <memory>
#include <string>

using namespace std;

int main(int argc , char**argv){
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

    pm->run();
    cout << irgen.getModule()->print();
    // pm->getInfo<Dominators>()->printDomSet();
    // pm->getInfo<Dominators>()->printDomFront();
    // pm->getInfo<Dominators>()->printDomTree();
    // delete (p);

}