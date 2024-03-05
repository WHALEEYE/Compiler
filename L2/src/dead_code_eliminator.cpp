#include <unordered_set>
#include <vector>

#include <L2.h>
#include <dead_code_eliminator.h>
#include <liveness_analyzer.h>

using namespace std;

namespace L2 {

class DeadCodeEliminator : Visitor {
public:
  void visit(const Register *reg) {
    if (reg->getID() == Register::ID::RSP)
      return;
    if (OUT->find(reg) == OUT->end())
      eliminated = true;
  }

  void visit(const Variable *var) {
    if (OUT->find(var) == OUT->end())
      eliminated = true;
  }

  void visit(const Number *num) {}
  void visit(const CompareOp *op) {}
  void visit(const ShiftOp *op) {}
  void visit(const ArithOp *op) {}
  void visit(const SelfModOp *op) {}
  void visit(const MemoryLocation *mem) {}
  void visit(const StackLocation *stack) {}
  void visit(const FunctionName *name) {}
  void visit(const Label *label) {}

  void visit(const RetInst *inst) {}

  void visit(const ShiftInst *inst) { inst->getLval()->accept(*this); }

  void visit(const ArithInst *inst) { inst->getLval()->accept(*this); }

  void visit(const SelfModInst *inst) { inst->getLval()->accept(*this); }

  void visit(const AssignInst *inst) { 
    if (inst->getLval() == inst->getRval()) {  
      eliminated = true;
      return;
    }
    
    inst->getLval()->accept(*this);
  }

  void visit(const CompareAssignInst *inst) { inst->getLval()->accept(*this); }

  void visit(const CallInst *inst) {}
  void visit(const PrintInst *inst) {}
  void visit(const InputInst *inst) {}
  void visit(const AllocateInst *inst) {}
  void visit(const TupleErrorInst *inst) {}
  void visit(const TensorErrorInst *inst) {}

  void visit(const SetInst *inst) { inst->getLval()->accept(*this); }

  void visit(const LabelInst *inst) {}
  void visit(const GotoInst *inst) {}
  void visit(const CondJumpInst *inst) {}

  bool visitBB(BasicBlock *BB) {
    changed = false;
    liveInsts.clear();
    for (auto inst : BB->getInstructions())
      visitInst(inst);
    return changed;
  }

  void visitInst(const Instruction *inst) {
    eliminated = false;
    OUT = &liveness.getLivenessSets(inst).getOUT();
    inst->accept(*this);
    changed |= eliminated;
    if (!eliminated)
      liveInsts.push_back(inst);
  }

  vector<const Instruction *> &getLiveInsts() { return liveInsts; }

  DeadCodeEliminator(const LivenessResult &liveness) : liveness(liveness) {}

private:
  vector<const Instruction *> liveInsts;

  const unordered_set<const Symbol *> *OUT;
  const LivenessResult &liveness;
  bool changed, eliminated;
};

bool doElimination(Function *F) {
  bool changed = false;
  auto &liveness = analyzeLiveness(F);
  DeadCodeEliminator eliminator(liveness);
  for (auto BB : F->getBasicBlocks()) {
    bool bbChanged = eliminator.visitBB(BB);
    changed |= bbChanged;

    if (!bbChanged)
      continue;

    auto &liveInsts = eliminator.getLiveInsts();
    BB->instructions = liveInsts;
  }
  return changed;
}

void eliminateDeadCode(Function *F) {
  while (doElimination(F)) {
  }
}

} // namespace L2