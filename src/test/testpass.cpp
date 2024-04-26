#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Module.hpp"
#include "frontend/parser.hpp"
#include <bitset>
#include <fstream>
#include <ios>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "midend/IRGen.hpp"

#include "analysis/Dominators.hpp"
#include "optimization/Mem2Reg.hpp"
#include "optimization/PassManager.hpp"
using namespace std;

int main(int argc , char**argv){
    if(argc<2){
        std::cerr<<("expect argv")<<endl;
        exit(-1);
    }
    Parser *p=new Parser(argv[1]);
    p->parserComp();
    IRBuilder::IRGen irgen;
    p->comp->accept(irgen);
    auto m=irgen.getModule();
 
    // string str_out=argv[1];
    // str_out=str_out+".ll";
    // fstream os;
    // os.open(str_out,ios_base::out);
    irgen.getModule()->print();
    PassManager pm{m};
    pm.add_pass<Mem2Reg>();
    pm.run();
    cout << irgen.getModule()->print();

    // for(auto f:m->getFunctions()){
    //     if(f->getBasicBlocks().size()>1){
    //         cout<<f->printGra()<<endl;
    //     }
    // }


    delete (p);

}