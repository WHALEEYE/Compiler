#include <iostream>
#include <map>
#include <queue>
#include <unordered_set>

#include <L2.h>
#include <liveness_analyzer.h>

namespace L2 {

bool setEqual(const LivenessSet &a, const LivenessSet &b) {
  if (a.size() != b.size())
    return false;

  for (auto var : a)
    if (b.find(var) == b.end())
      return false;

  return true;
}

void printResult(Function *F) {
  std::cout << "(" << std::endl << "(in" << std::endl;
  for (auto BB : F->getBasicBlocks())
    for (auto I : BB->getInstructions()) {
      auto &IN = I->getIN();
      std::cout << "(";
      for (auto pV = IN.begin(); pV != IN.end();) {
        std::cout << (*pV)->toStr();
        if (++pV != IN.end())
          std::cout << " ";
      }
      std::cout << ")" << std::endl;
    }

  std::cout << ")" << std::endl << std::endl << "(out" << std::endl;

  for (auto BB : F->getBasicBlocks())
    for (auto I : BB->getInstructions()) {
      auto &OUT = I->getOUT();
      std::cout << "(";
      for (auto pV = OUT.begin(); pV != OUT.end();) {
        std::cout << (*pV)->toStr();
        if (++pV != OUT.end())
          std::cout << " ";
      }
      std::cout << ")" << std::endl;
    }

  std::cout << ")" << std::endl << std::endl << ")" << std::endl;
}

bool analyzeInBB(BasicBlock *BB, bool visited) {
  LivenessSet buffer;
  for (auto succ : BB->getSuccessors()) {
    auto &succIN = succ->getFirstInstruction()->getIN();
    buffer.insert(succIN.begin(), succIN.end());
  }
  if (visited && setEqual(buffer, BB->getTerminator()->getOUT()))
    return false;

  for (auto pI = BB->getInstructions().rbegin(); pI != BB->getInstructions().rend(); pI++) {
    auto I = *pI;
    I->setOUT(buffer);
    auto &GEN = I->getGEN(), &KILL = I->getKILL();
    for (auto var : KILL)
      buffer.erase(var);
    buffer.insert(GEN.begin(), GEN.end());
    I->setIN(buffer);
  }
  return true;
}

void livenessAnalyze(Function *F) {
  std::queue<BasicBlock *> workq;
  std::map<BasicBlock *, bool> visited;
  for (auto pB = F->getBasicBlocks().rbegin(); pB != F->getBasicBlocks().rend(); pB++)
    workq.push(*pB);

  while (!workq.empty()) {
    auto BB = workq.front();
    workq.pop();
    if (analyzeInBB(BB, visited[BB])) {
      for (auto pred : BB->getPredecessors())
        workq.push(pred);
    }
    visited[BB] = true;
  }
}

} // namespace L2