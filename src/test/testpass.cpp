#include "frontend/parser.hpp"
#include <iostream>
#include <memory>
#include <string>
#include "midend/IRGen.hpp"

#include "optimization/DeadStoreEli.hpp"
#include "optimization/Mem2Reg.hpp"
#include "optimization/PassManager.hpp"
#include "optimization/LIR.hpp"
#include "optimization/inline.hpp"
#include "analysis/CIDBB.hpp"
#include "analysis/CLND.hpp"
#include "backend/AsmGen.hpp"
#include "backend/Asm.hpp"
#include "optimization/AAA.hpp"
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
  //  pm.add_pass<DeadStoreEli>();
    pm.add_pass<Mem2Reg>();
   // // pm.add_pass<ADCE>();
  // pm.run();
   
   pm.add_pass<LIR>();
   pm.add_pass<CIDBB>();
   pm.add_pass<CLND>();
   pm.add_pass<ActiveVar>();
    // pm.add_pass<FuncInline>();
    pm.run();
    cout << irgen.getModule()->print()<<endl;;
   AsmGen asm_gen(m);
    m->accept(asm_gen);
    ::std::string asm_code = asm_gen.getAsmUnit()->print();
    ::std::cout<<asm_code<<::std::endl;


    delete (p);

}