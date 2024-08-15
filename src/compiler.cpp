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
#include "optimization/ArrReduc.hpp"
#include "optimization/CombinBB.hpp"
#include "optimization/ConstBrEli.hpp"
#include "optimization/DCE.hpp"
#include "optimization/DFE.hpp"
#include "optimization/DeadPHIEli.hpp"
#include "optimization/DeadStoreEli.hpp"
#include "optimization/G2L.hpp"
#include "optimization/GenLoadImm.hpp"
#include "optimization/GepCombine.hpp"
#include "optimization/GepOpt.hpp"
#include "optimization/InstrCombine.hpp"
#include "optimization/Mem2Reg.hpp"
#include "optimization/MoveAlloca.hpp"
#include "optimization/PassManager.hpp"
#include "optimization/SCCP.hpp"
#include "optimization/ValueNumbering.hpp"
#include "optimization/VirtualRetEli.hpp"
#include "optimization/inline.hpp"
#include "optimization/instrResolve.hpp"

#include "analysis/LoopInfo.hpp"
#include "analysis/LoopInvariant.hpp"
#include "analysis/Dataflow.hpp"
#include "analysis/SCEV.hpp"

#include "optimization/LoopSimplified.hpp"
#include "optimization/LICM.hpp"
#include "optimization/LoopUnroll.hpp"
#include "optimization/LoopStrengthReduction.hpp"

#include "optimization/BreakGEP.hpp"
#include "optimization/CombineJJ.hpp"
#include "optimization/MemInstOffset.hpp"
#include "optimization/splitArr.hpp"
#include "analysis/CLND.hpp"

void usage(){
	(void)fprintf(stderr, "%s\n%s\n",
	    "usage: compiler file [-SL]  file -o file ...",
	    "usage: compiler file [-SL]  file -o file -O1...");
	exit(EXIT_FAILURE);
}
void Compiler::buildOpt(PassManager &pm){
    pm.addInfo<Dominators>();
    pm.addInfo<FuncAnalyse>();
    pm.addInfo<LiveVar>();
    pm.addInfo<LoopInfo>();
    pm.addInfo<LoopInvariant>();
    pm.addInfo<SCEV>();
    
    //mem2regpass
    pm.addPass<DeadStoreEli>();    
    pm.addPass<CombinBB>();
    pm.addPass<Mem2Reg>();
    pm.addPass<DeadPHIEli>();
    pm.addPass<SCCP>();
    pm.addPass<CombinBB>();
    pm.addPass<InstrCombine>();
    pm.addPass<SCCP>();
    pm.addPass<CombinBB>();
    pm.addPass<InstrCombine>();
    pm.addPass<InstrResolve>();
    pm.addPass<DCE>();
    pm.addPass<DFE>();

    //loop pass
    pm.addPass<LoopSimplified>();
    pm.addPass<LoopStrengthReduction>();
    pm.addPass<LICM>();
    pm.addPass<LoopUnroll>();
    pm.addPass<SCCP>();
    pm.addPass<CombinBB>();
    pm.addPass<DCE>();
    pm.addPass<InstrCombine>();

    //arrpass
    pm.addPass<MoveAlloca>();
    pm.addPass<ValNumbering>();
    pm.addPass<CombinBB>();
    pm.addPass<ArrReduc>();
    pm.addPass<SplitArr>();
    pm.addPass<Mem2Reg>();
    pm.addPass<DCE>();
    pm.addPass<SCCP>();

    //inline and g2l pass
    pm.addPass<FuncInline>();
    pm.addPass<CombinBB>();
    pm.addPass<G2L>();
    pm.addPass<DeadPHIEli>();
    pm.addPass<DCE>();
    pm.addPass<SCCP>();
    pm.addPass<InstrCombine>();
    pm.addPass<ConstBr>();
    pm.addPass<ValNumbering>();
    pm.addPass<ArrReduc>();
    pm.addPass<SCCP>();
    pm.addPass<CombinBB>();

    //gep loadimm and licm pass
    pm.addPass<BreakGEP>();
    pm.addPass<LoopSimplified>();
    pm.addPass<LICM>();

    pm.addPass<CombinBB>();
    // pm.addPass<SCCP>();
    pm.addPass<InstrCombine>();
    pm.addPass<DCE>();
    pm.addPass<GepCombine>();
    pm.addPass<DCE>();
    pm.addPass<ValNumbering>();
    pm.addPass<GepCombine>();

    pm.addPass<CombinBB>();
    // pm.addPass<VRE>();
    pm.addPass<DCE>();

    pm.addPass<GenLoadImm>();
    lir(pm);

    pm.addPass<GepOpt>();
    pm.addPass<DCE>();
    pm.addPass<ValNumbering>();
    pm.addPass<InstrReduc>();
}

void Compiler::buildDefault(PassManager &pm){
    pm.addInfo<Dominators>();
    pm.addPass<DeadStoreEli>();    
    pm.addPass<CombinBB>();
    pm.addPass<Mem2Reg>();
    pm.addPass<DeadPHIEli>();
    pm.addPass<SCCP>();
    pm.addPass<BreakGEP>();
    pm.addPass<GenLoadImm>();
    lir(pm);
    pm.addPass<DCE>();
    
}
Compiler::Compiler(int argc, char** argv):lir([](PassManager&pm){
    pm.addPass<CombineJJ>();
    pm.addPass<MemInstOffset>();
    pm.addPass<MoveAlloca>();}){
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
    m->setPrintName();

    std::fstream out_file(out_name,std::ios::out|std::ios::trunc);
    if(is_out_llvm){
        out_file<<m->print()<<std::endl;
    }else{
        AsmGen asm_gen(m);
        auto reg_alloc = new LSRA(m);
        pm.addInfo<LiveVar>();
        pm.addInfo<CIDBB>();
        pm.addInfo<CLND>();
        auto lv = pm.getInfo<LiveVar>();
        lv->analyse();
        auto cidbb = pm.getInfo<CIDBB>();
        cidbb->analyse();
        auto clnd = pm.getInfo<CLND>();
        clnd->analyse();
        reg_alloc->run(lv, cidbb, &asm_gen);
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
