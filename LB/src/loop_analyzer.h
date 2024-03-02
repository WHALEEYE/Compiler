#pragma once
#include <LB.h>
#include <unordered_map>

namespace LB {

class LoopInfo {
public:
  std::unordered_map<const Instruction *, const WhileInst *> loopMap;
  std::unordered_map<const WhileInst *, const Label *> loopLabels;
};

LoopInfo &analyzeLoops(Function *F);

} // namespace LB