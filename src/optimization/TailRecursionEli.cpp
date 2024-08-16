#include "analysis/Info.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "optimization/TailRecursionEli.hpp"
// 只处理尾递归 
Modify TailRecursionElim::runOnFunc(Function *func) {
    if (func->isDeclaration())
        return{};

    std::vector<CallInst *> recur_calls;
    for (auto BB: func->getBasicBlocks()) {
        auto ter=BB->getInstructions().back();
        if(!ter->isRet())continue;
        for (auto iter= BB->getInstructions().begin();iter!= BB->getInstructions().end();++iter) {
            if (const auto call = dynamic_cast<CallInst*>(*iter)) {
                if (call->getOperand(0) != func)
                    continue;
                auto retiter=++iter;
                if(*retiter!=ter)
                    return {};
                if(ter->getOperand(0)!=call)
                    return {};
                recur_calls.push_back(call);
            }
        }
    }

    if (recur_calls.empty())
        return {};

    const auto entry = func->getEntryBlock();
    const auto head = BasicBlock::create("",func);
    head->getInstructions() = std::move(entry->getInstructions());
    for (auto succ : entry->getSuccBasicBlocks()) {
        succ->removePreBasicBlock(entry);
        succ->addPreBasicBlock(head);
    }
    head->getSuccBasicBlocks() = std::move(entry->getSuccBasicBlocks());
    entry->replaceAllUseWith(head);
    BranchInst::createBr(head, entry);


    std::vector<PhiInst *> arg_phi;
    for (auto arg : func->getArgs()) {
        auto phi = PhiInst::createPhi(arg->getType(), head);
        arg->replaceAllUseWith(phi);
        head->addInstrBegin(phi);
        arg_phi.push_back(phi);
        phi->addPhiPairOperand(arg, entry);
    }

    for (auto call : recur_calls) {
        auto bb = call->getParent();
        auto &instr_list = bb->getInstructions();

        auto opds = call->getOperands();

        for (int i = 1; i < opds.size(); ++i) {
            auto phi = arg_phi[i - 1];
            phi->addPhiPairOperand(opds[i], bb);
        }

        instr_list.pop_back(); // ret
        instr_list.pop_back(); // call
        BranchInst::createBr(head, bb);
        call->removeUseOfOps();
        delete call;
    }
    Modify ret;
    ret.modify_bb=true;
    ret.modify_instr=true;
    ret.modify_call=true;
    return ret;
}
