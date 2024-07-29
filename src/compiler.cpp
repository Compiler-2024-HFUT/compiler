#include <cstdlib>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdio.h>

#include "analysis/Dominators.hpp"
#include "analysis/funcAnalyse.hpp"
#include "backend/AsmGen.hpp"
#include "frontend/parser.hpp"
#include "compiler.hpp"
#include "midend/IRGen.hpp"
#include "optimization/CombinBB.hpp"
#include "optimization/ConstBrEli.hpp"
#include "optimization/DCE.hpp"
#include "optimization/DeadPHIEli.hpp"
#include "optimization/DeadStoreEli.hpp"
#include "optimization/G2L.hpp"
#include "optimization/InstrCombine.hpp"
#include "optimization/Mem2Reg.hpp"
#include "optimization/MoveAlloca.hpp"
#include "optimization/PassManager.hpp"
#include "optimization/SCCP.hpp"
#include "optimization/ValueNumbering.hpp"
#include "optimization/inline.hpp"

#include "optimization/BreakGEP.hpp"
#include "optimization/CombineJJ.hpp"
#include "optimization/MemInstOffset.hpp"

void usage(){
	(void)fprintf(stderr, "%s\n%s\n",
	    "usage: compiler file [-SL]  file -o file ...",
	    "usage: compiler file [-SL]  file -o file -O1...");
	exit(EXIT_FAILURE);
}
void Compiler::buildOpt(PassManager &pm){
    pm.addInfo<Dominators>();
    pm.addInfo<FuncAnalyse>();
    pm.addPass<DeadStoreEli>();    
    pm.addPass<CombinBB>();
    pm.addPass<Mem2Reg>();
    pm.addPass<DeadPHIEli>();
    pm.addPass<DCE>();
    pm.addPass<SCCP>();
    pm.addPass<CombinBB>();
    pm.addPass<InstrCombine>();
    pm.addPass<FuncInline>();
    pm.addPass<CombinBB>();
    pm.addPass<G2L>();
    pm.addPass<DeadPHIEli>();
    pm.addPass<SCCP>();
    pm.addPass<InstrCombine>();
    pm.addPass<ConstBr>();
    pm.addPass<CombineJJ>();
    pm.addPass<BreakGEP>();
    pm.addPass<ValNumbering>();
    pm.addPass<CombinBB>();
    pm.addPass<InstrCombine>();
    pm.addPass<InstrReduc>();
    pm.addPass<MemInstOffset>();
    pm.addPass<MoveAlloca>();
    pm.addPass<DCE>();
}

void Compiler::buildDefault(PassManager &pm){
    pm.addInfo<Dominators>();
    pm.addPass<DeadStoreEli>();    
    pm.addPass<CombinBB>();
    pm.addPass<Mem2Reg>();
    pm.addPass<DeadPHIEli>();
    pm.addPass<SCCP>();
    pm.addPass<CombineJJ>();
    pm.addPass<BreakGEP>(); 
    pm.addPass<MemInstOffset>();  
    pm.addPass<MoveAlloca>();
    
}
Compiler::Compiler(int argc, char** argv){
    if(argc<5){
        usage();
    }
    int ch;
    int op=0;
    char* op_str;
    while ((ch = getopt(argc, argv, "o:O:SL")) != EOF) {
        switch (ch) {
        case 'o':
            out_name=optarg;
            break;
        case 'O':
            op_str=optarg;
            if(op_str==0||op_str[1]!=0)
                usage();
            op=op_str[0]-'0';
            if(op>2||op<0)
                usage();
            opt_level=(OPT)op;
            break;
        case 'S':
            is_out_asm=true;
            break;
        case 'L':
            is_out_llvm=true;
            break;             
        default:
            usage();
        }
    }
    if(!is_out_asm&&!is_out_llvm)
        usage();
    argv += optind;
    argc -= optind;
    if(!argv[0])
        usage();
    in_name=argv[0];
}
int Compiler::run(){
    IRgen::IRGen irgen;
    {
        Parser p{in_name};
        p.parserComp();
        p.comp->accept(irgen);
    }
    auto m=irgen.getModule();
    PassManager pm(m);
    if(opt_level!=OPT::NONE)
        buildOpt(pm);
    else
        buildDefault(pm);
    pm.run();
    m->print();

    std::fstream out_file(out_name,std::ios::out|std::ios::trunc);
    if(is_out_llvm){
        out_file<<m->print()<<std::endl;
    }else{
        AsmGen asm_gen(m);
        m->accept(asm_gen);
        ::std::string asm_code = asm_gen.getAsmUnit()->print();
        out_file<<asm_code<<std::endl;
    }
    out_file.close();

    return 0;
}
int main(int argc, char** argv)
{
    Compiler compiler{argc,argv} ;
    return compiler.run();
}
