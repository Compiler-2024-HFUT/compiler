#include "optimization/LoopParallel.hpp"

static void retargetBasicBlock(BasicBlock* target, BasicBlock* oldSource, BasicBlock* newSource) {
    for(auto& inst : target->getInstructions()) {
        if(inst->isPhi()) {
            const auto phi = dynamic_cast<PhiInst*>(inst);
            if(phi->incomings().count(newSource)) {
                assert(phi->incomings().at(oldSource) == phi->incomings().at(newSource));
                phi->removeSource(oldSource);
            } else
                phi->replaceSource(oldSource, newSource);
        } else
            break;
    }
}
static void resetTarget(Instruction* branchOrSwitch, BasicBlock* oldTarget, BasicBlock* newTarget) {
    const auto handleTarget = [=](BasicBlock *target) {
        if(target == oldTarget)
            target = newTarget;
    };

    // only br??
    // if(branchOrSwitch->getInstID() == InstructionID::Switch) {
    //     const auto switchInst = branchOrSwitch->as<SwitchInst>();
    //     handleTarget(switchInst->defaultTarget());
    //     for(auto& [val, target] : switchInst->edges()) {
    //         handleTarget(target);
    //     }
    // } else {
        const auto branch = dynamic_cast<BranchInst*>(branchOrSwitch);
        assert(branch->getOperand(1) == oldTarget || branch->getOperand(2) == oldTarget);
        handleTarget(dynamic_cast<BasicBlock*>(branch->getOperand(1)));
        handleTarget(dynamic_cast<BasicBlock*>(branch->getOperand(2)));
    // }
}

static bool matchAddRec(Value *giv, BasicBlock *latch, std::unordered_set<Value*>& values) {
    Instruction *givI = dynamic_cast<Instruction*>(giv);
    if(!givI || !givI->isPhi())
        return false;
    
    Value *givNext = nullptr;
    vector<Value*> ops = givI->getOperands();
    for(int i = 1; i < ops.size(); ++i) {
        if(ops[i] == latch)
            givNext = ops[i-1];
    }
    if(!givNext)
        return false;

    Instruction *givNextI = dynamic_cast<Instruction*>(givNext);
    if(givNextI) {
        Value* v2;
        if(givNextI->isPhi()) {
            
            vector<Value*> ops = givNextI->getOperands();
            for(int i = 1; i < ops.size(); ++i) {
                if(ops[i-1] == givI) {
                    continue;
                }
                PhiInst* base;
                Instruction *opI = dynamic_cast<Instruction*>(ops[i-1]);
                if( !opI->isAdd() )
                    return false;
                if( dynamic_cast<PhiInst*>(opI->getOperand(0)) ) {
                    base = dynamic_cast<PhiInst*>(opI->getOperand(0));
                    v2 = opI->getOperand(1);
                } else {
                    return false;
                }

                if(!matchAddRec(base, dynamic_cast<BasicBlock*>(ops[i]), values))
                    return false;
                values.insert(ops[i-1]);
                values.insert(base);
            }
            values.insert(givI);
            values.insert(givNext);
            return true;
        }

        Instruction *nextI = dynamic_cast<Instruction*>(givNext);
        if(nextI->isAdd() && nextI->getOperand(0) == givI) {
            values.insert(givI);
            values.insert(givNext);
            return true;
        } else {
            return false;
        }
    }

    return false;
}

bool LoopParallel::isNoSideEffectExpr(Instruction *inst) {
    if(inst->isTerminator() || inst->isStore())
        return false;
    if(inst->isCall() && inst->getType()->isVoidType())
        return false;

    if(inst->isCall()) {
        // pure function?
        Function *callee = dynamic_cast<Function*>( dynamic_cast<CallInst*>(inst)->getOperand(0) );
        if(!callee)
            return false;
        
        FuncAnalyse *funcA = info_man_->getInfo<FuncAnalyse>();
        if(!funcA->isPureFunc(callee))
            return false;

        // if(auto func = dynamic_cast<Function*>(callee)) {
        //     auto& attr = func->attr();
        //     return attr.hasAttr(FunctionAttribute::NoSideEffect) && attr.hasAttr(FunctionAttribute::Stateless) &&
        //         !attr.hasAttr(FunctionAttribute::NoReturn);
        // }
        // return false;
    }

    return true;
}

static Value *lookUpPtrBase(Instruction *inst) {
    if(inst->isStore() || inst->isLoad()) {
        Value *ptr =  (inst->isStore()) ? inst->getOperand(1) : inst->getOperand(0);
        GetElementPtrInst *gep = dynamic_cast<GetElementPtrInst*>(ptr);
        
        // has other???
        if(dynamic_cast<GlobalVariable*>(ptr))
            return ptr;
        else if(gep) {
            return gep->getOperand(0);
        }
    }
    assert(0);
}

bool LoopParallel::extractLoopBody(Function *func, Loop *loop, Module *mod, bool independent, bool allowInnermost,
                     bool allowInnerLoop, bool onlyAddRec, bool estimateBasicBlockSizeForUnroll, bool needSubLoop,
                     bool convertReduceToAtomic, bool duplicateCmp, LoopBodyInfo *ret) {
    if(!allowInnermost && loop->getHeader() == loop->getSingleLatch())
        return false;

    uint32_t phiCount = 0;
    for(auto& inst : loop->getHeader()->getInstructions())
        if(inst->isPhi())
            ++phiCount;
        else
            break;
    // at most 1 BIV + 1 GIV
    // FIXME: support more GIV
    if(phiCount > 2)
        return false;

    // get loop's indvar
    if(phiCount == 1) {
        loop->setIndVar( dynamic_cast<PhiInst*>(loop->getHeader()->getInstructions().front()) );
    } else {
        loop->setIndVar( dynamic_cast<PhiInst*>( *std::next(loop->getHeader()->getInstructions().begin()) ) );
    }

    // loop->getHeader()->dumpAsTarget(std::cerr);
    // std::cerr << '\n';
    // loop->getSingleLatch()->dumpAsTarget(std::cerr);
    // std::cerr << '\n';

    Dominators *dom = info_man_->getInfo<Dominators>();

    std::unordered_set<BasicBlock*> body = std::unordered_set<BasicBlock*>(loop->getBlocks());
    // latch -> exit
    for(auto block : body) {
        if(block == loop->getSingleLatch())
            continue;
        for(auto succ : block->getSuccBasicBlocks()) {
            if(!body.count(succ))
                return false;
        }
    }

    // std::cerr << "detect body\n";
    // for(auto b : body) {
    //     b->dump(std::cerr, Noop{});
    // }
    // std::cerr << '\n';

    // in parallel, it is false
    if(estimateBasicBlockSizeForUnroll) {
        ;
    }

    // 基本归纳变量（BIV）在循环内通过增加或减少, 普通归纳变量（GIV）是基本归纳变量的线性函数（GIV = a * BIV + b）。
    // new_loop:
    //   biv/giv
    //   loop_body: void loop_body(i, inv...)/giv = loop_body(i, giv, inv...)
    //   next
    //   cmp
    //   br cmp, new_loop, exit

    // use in inner?
    Value* giv = nullptr;
    bool givUsedByOuter = false;
    for(auto& inst : loop->getHeader()->getInstructions())
        if(inst->isPhi()) {
            if(inst != loop->getIndVar()) {
                giv = inst;
            }
        } else
            break;
    if(giv) {
        vector<Value*> vals = {giv};
        vector<Value*> ops = dynamic_cast<PhiInst*>(giv)->getOperands();
        for(int i = 1; i < ops.size(); ++i) {
            if(ops[i] == loop->getSingleLatch()) {
                vals.push_back(ops[i-1]);
            }
        }

        for(auto inst : vals) {
            Instruction *instI = dynamic_cast<Instruction*>(inst);
            if(!instI)
                continue;
            for(auto use : instI->getUseList()) {
                Value *user = use.val_;
                if(!body.count(dynamic_cast<Instruction*>(user)->getParent())) {
                    givUsedByOuter = true;
                    break;
                }
            }
            if(givUsedByOuter)
                break;
        }
    }

    Value *givNext;
    vector<Value*> ops = dynamic_cast<Instruction*>(giv)->getOperands();
    for(int i = 1; i < ops.size(); ++i) {
        if(ops[i] == loop->getSingleLatch())
            givNext = ops[i-1];
    }
    Instruction *givNextI = dynamic_cast<Instruction*>(givNext);
    if(!givNextI)
        return false;

    // use in outer?
    bool givUsedByInner = false;
    Value* givAddRecInnerStep = nullptr;
    if(giv && onlyAddRec) {
        // std::cerr << "matching\n";
        std::unordered_set<Value*> values;
        if(!matchAddRec(giv, loop->getSingleLatch(), values))
            return false;
        for(auto inst : values) {
            for(auto use : inst->getUseList()) {
                Value *user = use.val_;
                if(!values.count(user) && body.count(dynamic_cast<Instruction*>(user)->getParent())) {
                    givUsedByInner = true;
                    break;
                }
            }
        }
        if(givUsedByInner) {
            if(givUsedByOuter)
                return false;

            // The initial value of giv should be inferred from the indvar
            Value* v2;
            // add(exactly(giv), any(v2))(MatchContext<Value>{ givNext }
            if(givNextI->isAdd() && givNextI->getOperand(0) == giv) {
                // scev addrec
                Instruction *v2I = dynamic_cast<Instruction*>(v2);

                if(!v2I || v2I->getParent() && (v2I->getParent() == loop->getHeader() || !dom->isLdomR(v2I->getParent(), loop->getHeader())))
                    return false;
                givAddRecInnerStep = v2;
            }
            else
                return false;
        }
    }
    std::unordered_set<Value*> allowedToBeUsedByOuter;
    allowedToBeUsedByOuter.insert(loop->getIndVar());

    Instruction *branch = loop->getHeader()->getTerminator();
    Instruction *cond = dynamic_cast<Instruction*>(branch->getOperand(0));
    if(!cond->isCmp())
        return false;
    const auto cmp = dynamic_cast<CmpInst*>(cond);
    if(!cmp->isCmp())
        return false;
    const auto loopnext = cmp->getOperand(0);

    allowedToBeUsedByOuter.insert(loopnext);
    if(giv) {
        allowedToBeUsedByOuter.insert(giv);
        allowedToBeUsedByOuter.insert(givNext);
    }

    for(auto block : body) {
        for(auto& inst : block->getInstructions()) {
            if(allowedToBeUsedByOuter.count(inst))
                continue;
            for(auto use : inst->getUseList()) {
                Value *user = use.val_;
                if(body.count(dynamic_cast<Instruction*>(user)->getParent()))
                    continue;
                return false;
            }
        }
    }

    if(independent) {
        std::unordered_map<Value*, uint32_t> loadStoreMap;
        for(auto b : body)
            for(auto& inst : b->getInstructions()) {
                if(inst->isTerminator())
                    continue;
                if(inst->isLoad() || inst->isStore()) {
                    const auto base = lookUpPtrBase(inst);
                    if(!base) {
                        return false;
                    }

                    loadStoreMap[base] |= (inst->isLoad() ? 1 : 2);
                } else if(!isNoSideEffectExpr(inst)) {
                    return false;
                }
            }
        std::vector<std::pair<Instruction*, Instruction*>> workList;
        
        /* don't add atomic add now
        for(auto [k, v] : loadStoreMap) {
            if(v == 3) {
                if(convertReduceToAtomic) {
                    // match load-store pair
                    // FIXME: check pointer aliasing
                    // v0 = load ptr
                    // v1 = v0 + inc
                    // store v1, ptr
                    // ==>
                    // atomicadd ptr, inc
                
                    Instruction* load = nullptr;
                    for(auto b : body)
                        for(auto& inst : b->getInstructions()) {
                            if(inst->isLoad() || inst->isStore()) {
                                const auto base = lookUpPtrBase(inst);
                                if(base == k) {
                                    if(inst->isLoad()) {
                                        if(load)
                                            return false;
                                        load = inst;
                                    } else {
                                        if(!load)
                                            return false;
                                        const auto store = inst;
                                        if(load->getOperand(0) == store->getOperand(1) && load->getUseList().size()==1 &&
                                           load->getParent() == store->getParent()) {
                                            const auto val = store->getOperand(0);
                                            Value *loadVal = load, *rhs;
                                            // oneUse(add(exactly(loadVal), any(rhs)))(MatchContext<Value>{ val })
                                            if( val->getUseList().size() == 1 && dynamic_cast<Instruction*>(val)->getOperand(0) == loadVal ) {
                                                workList.emplace_back(load, store);
                                                load = nullptr;
                                            } else
                                                return false;
                                        } else
                                            return false;
                                    }
                                }
                            }
                        }
                    if(load)
                        return false;
                    continue;
                }
                return false;
            }
        }

        for(auto [load, store] : workList) {
            const auto block = load->getParent();
            const auto ptr = store->getOperand(1);
            const auto val = dynamic_cast<Instruction*>(store->getOperand(0));
            const auto inc = val->getOperand(0) == load ? val->getOperand(1) : val->getOperand(0);
            const auto atomicAdd = make<AtomicAddInst>(ptr, inc);
            auto& insts = block->getInstructions();
            insts.erase(load->asNode());
            insts.erase(val->asNode());
            atomicAdd->insertBefore(block, store->asIterator());
            insts.erase(store->asNode());
        }
        */
    }

    auto getUniqueID = [&] {
        auto base = string("xc_loop_body_");
        for(int32_t id = 0;; ++id) {
            const auto name = base + std::to_string(id);
            bool used = false;
            // global is global vars???
            for(auto global : mod->getGlobalVariables()) {
                if(global->getName() == name) {
                    used = true;
                    break;
                }
            }
            if(!used)
                return name;
        }
    };
    auto funcType = FunctionType::get(giv ? giv->getType() : Type::getVoidType(), {});
    const auto bodyFunc = Function::create(funcType, getUniqueID(), mod);
    // bodyFunc->setLinkage(Linkage::Internal);
    // bodyFunc->attr().addAttr(FunctionAttribute::NoRecurse).addAttr(FunctionAttribute::LoopBody);
    // mod.add(bodyFunc);

    std::unordered_map<Value*, Value*> val2arg;
    val2arg.emplace(loop->getIndVar(), bodyFunc->addArg(loop->getIndVar()->getType()));
    if(giv) {
        val2arg.emplace(giv, bodyFunc->addArg(giv->getType()));
    }
    // duplicate cmp
    if(duplicateCmp)
        for(auto block : body) {
            const auto terminator = block->getTerminator();
            // terminator->getInstID() != InstructionID::ConditionalBranch
            if(terminator->isBr() && terminator->getNumOperands() == 1)
                continue;
            const auto cond = terminator->getOperand(0);
            Instruction *condI = dynamic_cast<Instruction*>(cond);
            if(!condI)
                return false;
            if(body.count(condI->getParent()))
                continue;
            // icmp(cmp, any(v1), int_(i1))(MatchContext<Value>{ cond })
            if( dynamic_cast<ConstantInt*>(condI->getOperand(1)) ) {
                // newCond->insertBefore(block, terminator->asIterator());
                const auto newCond = condI->copyInst(terminator->getParent());
                block->insertInstr(std::prev(block->getInstructions().end()), newCond);

                terminator->replaceOperand(0, newCond);
            }
        }
    for(auto block : body) {
        for(auto& inst : block->getInstructions()) {
            for(auto operand : inst->getOperands()) {
                if(val2arg.count(operand))
                    continue;
                if(dynamic_cast<Constant*>(operand) || dynamic_cast<GlobalVariable*>(operand))
                    continue;
                if(body.count(dynamic_cast<Instruction*>(operand)->getParent()))
                    continue;
                val2arg.emplace(operand, bodyFunc->addArg(operand->getType()));
            }
        }
    }
    // bodyFunc->updateTypeFromArgs();

    std::unordered_map<Value*, Value*> arg2Val;
    std::vector<Value*> args;
    for(auto [k, v] : val2arg) {
        arg2Val.emplace(v, k);
        // TrackableValue = Instruction, Argument, GlobalVariable
        // auto track = k->as<TrackableValue>();
        auto track = k;
        for(auto it = track->getUseList().begin(); it != track->getUseList().end();) {
            auto next = std::next(it);
            Value *user = (*it).val_;
            if(body.count( dynamic_cast<Instruction*>(user)->getParent()) ) {
                --
                // it.ref()->resetValue(v);

            }
            it = next;
        }
    }

    std::vector<Value*> callArgs;
    for(auto arg : bodyFunc->getArgs())
        callArgs.push_back(arg2Val.at(arg));

    bodyFunc->getBasicBlocks().push_back(loop->getHeader());
    for(auto block : body) {
        block->setParent(bodyFunc);
        if(block != loop->getHeader())
            bodyFunc->getBasicBlocks().push_back(block);
    }
    func->getBasicBlocks().remove_if([&](BasicBlock* block) { return body.count(block); });
    // IRBuilder builder{ mod.getTarget() };
    const auto oldCond = loop->getSingleLatch()->getTerminator()->getOperand(0);
    // const auto prob = loop->getSingleLatch()->getTerminator()->as<BranchInst>()->getBranchProb();
    const auto exit = dynamic_cast<BranchInst*>(loop->getSingleLatch()->getTerminator())->getOperand(2);
    loop->getSingleLatch()->getInstructions().pop_back();
    // builder.setCurrentBasicBlock(loop->getSingleLatch());
    if(giv)
        // builder.makeOp<ReturnInst>(givNext);
        ReturnInst::createRet(givNext, loop->getSingleLatch());
    else
        // builder.makeOp<ReturnInst>();
        ReturnInst::createVoidRet(loop->getSingleLatch());

    // builder.setCurrentFunction(&func);
    auto newLoop = BasicBlock::create("", func);
    for(auto it = loop->getHeader()->getInstructions().begin(); it != loop->getHeader()->getInstructions().end();) {
        auto& inst = *it;
        if(inst->isPhi()) {
            const auto next = std::next(it);
            inst.insertBefore(newLoop, newLoop->getInstructions().begin());
            it = next;
        } else
            break;
    }
    for(auto pred : loop->getHeader()->getPreBasicBlocks()) {
        if(pred != loop->getSingleLatch())
            resetTarget(dynamic_cast<BranchInst*>(pred->getTerminator()), loop->getHeader(), newLoop);
    }
    for(auto succ : loop->getSingleLatch()->getSuccBasicBlocks()) {
        if(succ != loop->getHeader())
            retargetBasicBlock(succ, loop->getSingleLatch(), newLoop);
    }

    // builder.setCurrentBasicBlock(newLoop);
    const auto call = CallInst::createCall(bodyFunc, callArgs, newLoop);
    const auto next = dynamic_cast<Instruction*>(loopnext)->copyInst(newLoop);
    // next->insertBefore(newLoop, newLoop->getInstructions().end());
    newLoop->insertInstr(newLoop->getInstructions().end(), next);
    const auto cond = dynamic_cast<Instruction*>(oldCond)->copyInst(newLoop);
    // cond->insertBefore(newLoop, newLoop->getInstructions().end());
    newLoop->insertInstr(newLoop->getInstructions().end(), cond);
    // builder.setCurrentBasicBlock(newLoop);
    // builder.makeOp<BranchInst>(cond, prob, newLoop, exit);
    BranchInst::createCondBr(cond, newLoop, dynamic_cast<BasicBlock*>(exit), newLoop);

    loop->getIndVar()->removeSource(loop->getSingleLatch());
    loop->getIndVar()->addIncoming(newLoop, next);
    if(giv) {
        giv->as<PhiInst>()->removeSource(loop->getSingleLatch());
        giv->as<PhiInst>()->addIncoming(newLoop, call);
    }
    for(auto inst : std::initializer_list<Instruction*>{ cond, next, loop->getIndVar(),
                                                         giv ? dynamic_cast<Instruction*>(giv) : nullptr }) {
        if(!inst)
            continue;
        for(auto& operand : inst->getOperands()) {
            if(auto it = arg2Val.find(operand); it != arg2Val.end()) {
                operand->resetValue(it->second);
            }
            if(operand == loopnext)
                operand->resetValue(next);
        }
    }

    for(auto val : allowedToBeUsedByOuter) {
        if(val == loop->getIndVar() || val == giv)
            continue;
        if(!dynamic_cast<Instruction*>(val))
            continue;
        const auto tracked = val->as<TrackableValue>();
        Value* rep = val == loopnext ? next : call;
        for(auto it = tracked->users().begin(); it != tracked->users().end();) {
            auto nextIt = std::next(it);
            auto block = it.ref()->user->getParent();
            if(block != newLoop && !body.count(block)) {
                it.ref()->resetValue(rep);
            }
            it = nextIt;
        }
    }

    if(ret) {
        ret->loop = newLoop;
        ret->indvar = loop->getIndVar();
        ret->bound = loop.bound;
        ret->rec = (giv ? dynamic_cast<PhiInst*>(giv) : nullptr);
        ret->recUsedByOuter = givUsedByOuter;
        ret->recUsedByInner = givUsedByInner;
        ret->recInnerStep = givAddRecInnerStep;
        ret->recNext = call;
        ret->exit = exit;
    }
    return true;
}