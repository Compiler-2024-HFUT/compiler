#include "optimization/LoopStrengthReduction.hpp"

void LoopStrengthReduction::visitLoop(Loop *loop) {

}

Modify LoopStrengthReduction::runOnFunc(Function* func) {
    Modify mod{};

    mod.modify_instr = true;
    return mod;
}