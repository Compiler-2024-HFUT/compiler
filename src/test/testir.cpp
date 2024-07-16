#include "frontend/parser.hpp"
#include <iostream>
#include <memory>
#include <string>
#include "midend/IRGen.hpp"
#include "midend/Module.hpp"
#include "optimization/Mem2Reg.hpp"
#include "optimization/DeadStoreEli.hpp"
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
 //   pm.add_pass<DeadStoreEli>();
    pm.add_pass<Mem2Reg>();
   // // pm.add_pass<ADCE>();
  // pm.run();
   
 
    // pm.add_pass<FuncInline>();
    pm.run();
    // string str_out=argv[1];
    // str_out=str_out+".ll";
    // fstream os;
    // os.open(str_out,ios_base::out);
 
    cout << irgen.getModule()->print();

    // auto fs=m->getFunctions();
    // for(auto f:fs){
    //     f->setInstrName();
    //     // cout<<f->getName()<<endl;
    //     for(auto bb:f->getBasicBlocks()){
    //         cout<<"cur: "<<bb->getName()<<" sb::";
    //         for(auto sb:bb->getSuccBasicBlocks()){
    //             cout<<sb->getName();
    //         }
    //         cout<<endl;
    //     }
    // }
    delete (p);

}