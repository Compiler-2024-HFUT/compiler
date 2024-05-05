#include "frontend/parser.hpp"
#include <iostream>
#include <memory>
#include <string>
#include "midend/IRGen.hpp"

#include "analysis/Dominators.hpp"
// #include "optimization/ADCE.hpp"
#include "optimization/DeadStoreEli.hpp"
#include "optimization/Mem2Reg.hpp"
#include "optimization/PassManager.hpp"
#include "optimization/SCCP.hpp"
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
 
    // string str_out=argv[1];
    // str_out=str_out+".ll";
    // fstream os;
    // os.open(str_out,ios_base::out);
    PassManager pm{m};
    pm.add_pass<DeadStoreEli>();
    pm.add_pass<Mem2Reg>();
    pm.add_pass<SCCP>();
    // pm.add_pass<ADCE>();
    pm.run();
    cout << irgen.getModule()->print();

<<<<<<< HEAD
=======
    // for(auto f:m->getFunctions()){
    //     if(f->getBasicBlocks().size()>1){
    //         cout<<f->printGra()<<endl;
    //     }
    // }

>>>>>>> b3b9b5e (仅能完成二元运算的传播)
    delete (p);

}