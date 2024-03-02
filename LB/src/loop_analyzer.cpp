#include <unordered_map>
#include <vector>

#include <LB.h>
#include <loop_analyzer.h>

using namespace std;

namespace LB {
LoopInfo &analyzeLoops(Function *F) {
  auto &loopInfo = *(new LoopInfo());
  unordered_map<const Label *, WhileInst *> bodyLabels, exitLabels;
  // collect information of while loops

  vector<int> loopPositions;
  auto &insts = F->getInstructions();
  for (int i = 0; i < insts.size(); i++) {
    auto I = insts[i];
    if (auto *whileInst = dynamic_cast<WhileInst *>(I)) {
      bodyLabels[whileInst->bodyLabel] = whileInst;
      exitLabels[whileInst->exitLabel] = whileInst;
      loopPositions.insert(loopPositions.begin(), i);
      // generate a new label for the while loop
      loopInfo.loopLabels[whileInst] = F->generateNewLabel();
    }
  }

  // add label before each while loop
  for (auto &pos : loopPositions) {
    auto whileInst = dynamic_cast<WhileInst *>(insts[pos]);
    auto labelInst = new LabelInst(loopInfo.loopLabels[whileInst]);
    insts.insert(insts.begin() + pos, labelInst);
  }

  vector<WhileInst *> loopStack;

  // map the instructions to while loops
  for (auto I : F->getInstructions()) {
    if (!loopStack.empty())
      loopInfo.loopMap[I] = loopStack.back();

    if (!dynamic_cast<LabelInst *>(I))
      continue;

    auto inst = dynamic_cast<LabelInst *>(I);
    if (bodyLabels.find(inst->getLabel()) != bodyLabels.end())
      loopStack.push_back(bodyLabels[inst->getLabel()]);
    else if (exitLabels.find(inst->getLabel()) != exitLabels.end())
      loopStack.pop_back();
  }

  return loopInfo;
};
} // namespace LB