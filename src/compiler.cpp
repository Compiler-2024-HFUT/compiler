#include <cstdlib>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdio.h>

#include "analysis/Dominators.hpp"
#include "frontend/parser.hpp"
#include "compiler.hpp"
#include "midend/IRGen.hpp"
#include "optimization/CombinBB.hpp"
#include "optimization/DeadPHIEli.hpp"
#include "optimization/DeadStoreEli.hpp"
#include "optimization/G2L.hpp"
#include "optimization/Mem2Reg.hpp"
#include "optimization/PassManager.hpp"
#include "optimization/SCCP.hpp"
#include "optimization/ValueNumbering.hpp"
void usage(){
	(void)fprintf(stderr, "%s\n%s\n",
	    "usage: compiler file [-SL]  file -o file ...",
	    "usage: compiler file [-SL]  file -o file -O1...");
	exit(EXIT_FAILURE);
}
void Compiler::buildOpt(PassManager &pm){
    pm.addInfo<Dominators>();
    pm.addPass<DeadStoreEli>();    
    pm.addPass<CombinBB>();
    pm.addPass<Mem2Reg>();
    pm.addPass<G2L>();
    pm.addPass<DeadPHIEli>();
    pm.addPass<SCCP>();
    // pm.addPass<ConstBr>();
    pm.addPass<CombinBB>();

    // pm.addPass<FuncInline>();
    pm.addPass<ValNumbering>();
}

void Compiler::buildDefault(PassManager &pm){
    pm.addInfo<Dominators>();
    pm.addPass<DeadStoreEli>();    
    pm.addPass<CombinBB>();
    pm.addPass<Mem2Reg>();
}
Compiler::Compiler(int argc, char** argv){
    if(argc<5){
        usage();
    }
    int ch;
    int op=0;
    while ((ch = getopt(argc, argv, "o:O:SL")) != EOF) {
        switch (ch) {
        case 'o':
            out_name=optarg;
            break;
        case 'O':
            op=atoi(optarg);
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
    PassManager pm(irgen.getModule());
    if(opt_level!=OPT::NONE)
        buildOpt(pm);
    else
        buildDefault(pm);
    pm.run();

    if(is_out_llvm){
        std::fstream out_file(out_name,std::ios::out|std::ios::trunc);
        out_file<<irgen.getModule()->print()<<std::endl;
        out_file.close();
    }else{
        
    }
    return 0;
}
int main(int argc, char** argv)
{
    Compiler compiler{argc,argv} ;
    return compiler.run();
}
