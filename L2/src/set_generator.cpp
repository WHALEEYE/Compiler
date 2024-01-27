#include "L2.h"
#include <cstdint>
#include <set_generator.h>
#include <vector>

namespace L2 {

class SetGenerator : public Visitor {
public:
  void visit(Register *reg) {
    debug("visiting register " + reg->toStr());
    if (reg == Register::getRegister(Register::ID::RSP))
      return;
    now->insert(reg);
  }

  void visit(Variable *var) {
    debug("visiting variable " + var->toStr());
    now->insert(var);
  }

  void visit(Number *num) {}
  void visit(CompareOp *op) {}
  void visit(ShiftOp *op) {}
  void visit(ArithOp *op) {}
  void visit(SelfModOp *op) {}

  void visit(MemoryLocation *mem) {
    // no matter how mem loc is accessed, the base register is always brought alive
    auto original = now;
    now = &GEN;
    mem->getBase()->accept(*this);
    now = original;
  }
  void visit(StackLocation *stack) {}
  void visit(FunctionName *name) {}
  void visit(Label *label) {}

  void visit(RetInst *inst) {
    GEN.insert(Register::getRegister(Register::ID::RAX));
    auto &calleeSaved = Register::getCalleeSavedRegisters();
    GEN.insert(calleeSaved.begin(), calleeSaved.end());
  }

  void visit(ShiftInst *inst) {
    // no need to delete lval, gonna be inserted anyway
    now = &GEN;
    inst->getLval()->accept(*this);
    inst->getRval()->accept(*this);
  }

  void visit(ArithInst *inst) {
    // no need to delete lval, gonna be inserted anyway
    now = &GEN;
    inst->getLval()->accept(*this);
    inst->getRval()->accept(*this);
  }

  void visit(SelfModInst *inst) {
    now = &GEN;
    inst->getLval()->accept(*this);
  }

  void visit(AssignInst *inst) {
    now = &KILL;
    inst->getLval()->accept(*this);
    now = &GEN;
    inst->getRval()->accept(*this);
  }

  void visit(CompareAssignInst *inst) {
    now = &KILL;
    inst->getLval()->accept(*this);
    now = &GEN;
    inst->getCmpLval()->accept(*this);
    inst->getCmpRval()->accept(*this);
  }

  void visit(CallInst *inst) {
    handleCall(inst->getArgNum()->getVal());
    now = &GEN;
    inst->getCallee()->accept(*this);
  }

  void visit(PrintInst *inst) { handleCall(1); }

  void visit(InputInst *inst) { handleCall(0); }

  void visit(AllocateInst *inst) { handleCall(2); }

  void visit(TupleErrorInst *inst) { handleCall(3); }

  void visit(TensorErrorInst *inst) { handleCall(inst->getArgNum()->getVal()); }

  void visit(SetInst *inst) {
    now = &KILL;
    inst->getLval()->accept(*this);
    now = &GEN;
    inst->getBase()->accept(*this);
    inst->getOffset()->accept(*this);
  }
  void visit(LabelInst *inst) {}

  void visit(GotoInst *inst) {}

  void visit(CondJumpInst *inst) {
    now = &GEN;
    inst->getLval()->accept(*this);
    inst->getRval()->accept(*this);
  }

  void doVisit(Instruction *inst) {
    GEN.clear();
    KILL.clear();
    inst->accept(*this);
    inst->setGEN(GEN);
    inst->setKILL(KILL);
  }

private:
  LivenessSet GEN, KILL;
  LivenessSet *now;

  // used as a readonly buffer
  const std::unordered_set<Register *> &callerSaved = Register::getCallerSavedRegisters(),
                                       &calleeSaved = Register::getCalleeSavedRegisters();
  const std::vector<Register *> &args = Register::getArgRegisters();

  void handleCall(int64_t argNum) {
    KILL.insert(callerSaved.begin(), callerSaved.end());
    for (int i = 0; i < std::min(argNum, (int64_t)6); i++)
      GEN.insert(args[i]);
  }
};

SetGenerator generator;

void generateSet(Function *F) {
  for (auto BB : F->getBasicBlocks())
    for (auto I : BB->getInstructions())
      generator.doVisit(I);
}

} // namespace L2