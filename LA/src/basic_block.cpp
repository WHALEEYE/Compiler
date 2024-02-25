#include <unordered_map>
using namespace std;

#include <LA.h>
#include <basic_block.h>
#include <helper.h>

namespace LA {

void formatBasicBlock(Function *F) {
  if (F->basicBlocks.back()->empty() && F->basicBlocks.size() > 1)
    F->basicBlocks.pop_back();

  // check if the first instruction is a label
  // if not, set a new label
  unordered_map<const Label *, BasicBlock *> labelToBB;
  for (auto BB : F->basicBlocks) {
    if (!BB->getLabel()) {
      auto newLabel = F->generateNewLabel();
      BB->setLabel(newLabel);
    }
    labelToBB[BB->getLabel()] = BB;
  }

  // link the basic blocks
  for (int i = 0; i < F->basicBlocks.size(); i++) {
    auto BB = F->basicBlocks[i];
    if (!BB->getTerminator()) {
      if (i == F->basicBlocks.size() - 1) {
        TerminatorInst *retInst;
        if (F->returnType == VoidType::getInstance())
          retInst = new RetInst();
        else
          retInst = new RetValueInst(new Number(0));
        BB->setTerminator(retInst);
      } else {
        auto nextBB = F->basicBlocks[i + 1];
        auto nextLabel = nextBB->getLabel();
        auto terminator = new BranchInst(nextLabel);
        BB->setTerminator(terminator);
      }
    }

    auto terminator = BB->getTerminator();
    if (auto branchInst = dynamic_cast<const BranchInst *>(terminator)) {
      auto target = labelToBB[branchInst->getLabel()];
      BB->addSuccessor(target);
      target->addPredecessor(BB);
    } else if (auto condBranchInst = dynamic_cast<const CondBranchInst *>(terminator)) {
      auto trueTarget = labelToBB[condBranchInst->getTrueLabel()];
      auto falseTarget = labelToBB[condBranchInst->getFalseLabel()];
      BB->addSuccessor(trueTarget);
      BB->addSuccessor(falseTarget);
      trueTarget->addPredecessor(BB);
      falseTarget->addPredecessor(BB);
    }
  }
}

} // namespace LA