/*
 * 1. 在该文件中完成对PassManager的构建，然后直接在main.cpp中调用
 * 2. 需要include的pass头文件也写在这里
 * 3. 添加info pass不需要考虑顺序，但是opt pass需要，排序手工完成（暂定）
 * 4. ... 
 */

#ifndef PASS_MANAGER_BUILDER_HPP
#define PASS_MANAGER_BUILDER_HPP

#include "analysis/InfoManager.hpp"
#include "optimization/PassManager.hpp"

// Info Pass
#include "analysis/LoopInfo.hpp"
#include "analysis/LoopInfo.hpp"

// Opt Pass
#include "optimization/Mem2Reg.hpp"

void buildTestPassManager(PassManager *pm) {

}

void buildDefaultPassManager(PassManager *pm) {

}

void buildAggressivePassManager(PassManager *pm) {

}



#endif