#include "liveness_analyzer.h"
#include <L2.h>
#include <spiller.h>
#include <stdexcept>

namespace L2 {
FunctionToSpill::FunctionToSpill(std::string name) : Function(name) { spilled = false; }

bool FunctionToSpill::getSpilled() const { return spilled; }
void FunctionToSpill::setSpilled(bool spilled) { this->spilled = spilled; }
const Variable *FunctionToSpill::getSpilledVar() const { return spilledVar; }
void FunctionToSpill::setSpilledVar(Variable *spilledVar) { this->spilledVar = spilledVar; }
std::string FunctionToSpill::getSpillPrefix() const { return spillPrefix; }
void FunctionToSpill::setSpillPrefix(std::string spillPrefix) { this->spillPrefix = spillPrefix; }

class Spiller : Visitor {
public:
  static Spiller *getInstance() {
    if (!instance)
      instance = new Spiller();
    return instance;
  }

  // when a item is visited, spill every item possible that equals to spilledVar
  // then store the updated version in buffer
  void visit(Register *reg) override { spilledItem = reg; }

  void visit(Variable *var) override {
    if (var != spilledVar) {
      spilledItem = var;
      return;
    }

    if (!newVar)
      newVar = new Variable(spillPrefix + std::to_string(spillCount++));
    spilledItem = newVar;
  }

  void visit(Number *num) override { spilledItem = num; }
  void visit(CompareOp *op) override { throw std::runtime_error("unexpected branch reached"); }
  void visit(ShiftOp *op) override { throw std::runtime_error("unexpected branch reached"); }
  void visit(ArithOp *op) override { throw std::runtime_error("unexpected branch reached"); }
  void visit(SelfModOp *op) override { throw std::runtime_error("unexpected branch reached"); }

  void visit(MemoryLocation *mem) override {
    mem->getBase()->accept(*this);
    auto base = (Symbol *)spilledItem;
    spilledItem = new MemoryLocation(base, mem->getOffset());
  }

  void visit(StackLocation *stack) override {
    throw std::runtime_error("unexpected branch reached");
  }

  void visit(FunctionName *name) override { spilledItem = name; }
  void visit(Label *label) override { spilledItem = label; }

  // when a instruction is visited, spill every item possible that equals to spilledVar
  // and store the updated version in spilledInst
  void visit(RetInst *inst) override { throw std::runtime_error("unexpected branch reached"); }

  void visit(ShiftInst *inst) override {
    inst->getLval()->accept(*this);
    auto lval = (Symbol *)spilledItem;
    inst->getRval()->accept(*this);
    auto rval = (Value *)spilledItem;
    spilledInst = new ShiftInst(inst->getOp(), lval, rval);
  }

  void visit(ArithInst *inst) override {
    inst->getLval()->accept(*this);
    auto lval = (Symbol *)spilledItem;
    inst->getRval()->accept(*this);
    auto rval = (Value *)spilledItem;
    spilledInst = new ArithInst(inst->getOp(), lval, rval);
  }

  void visit(SelfModInst *inst) override {
    inst->getLval()->accept(*this);
    auto lval = (Symbol *)spilledItem;
    spilledInst = new SelfModInst(inst->getOp(), lval);
  }

  void visit(AssignInst *inst) override {
    inst->getLval()->accept(*this);
    auto lval = (Symbol *)spilledItem;
    inst->getRval()->accept(*this);
    auto rval = (Value *)spilledItem;
    spilledInst = new AssignInst(lval, rval);
  }

  void visit(CompareAssignInst *inst) override {
    inst->getLval()->accept(*this);
    auto lval = (Symbol *)spilledItem;
    inst->getCmpLval()->accept(*this);
    auto cmpLval = (Value *)spilledItem;
    inst->getCmpRval()->accept(*this);
    auto cmpRval = (Value *)spilledItem;
    spilledInst = new CompareAssignInst(lval, inst->getOp(), cmpLval, cmpRval);
  }

  void visit(CallInst *inst) override {
    inst->getCallee()->accept(*this);
    auto callee = (Item *)spilledItem;
    spilledInst = new CallInst(callee, inst->getArgNum());
  }

  void visit(PrintInst *inst) override { throw std::runtime_error("unexpected branch reached"); }
  void visit(InputInst *inst) override { throw std::runtime_error("unexpected branch reached"); }
  void visit(AllocateInst *inst) override { throw std::runtime_error("unexpected branch reached"); }
  void visit(TupleErrorInst *inst) override {
    throw std::runtime_error("unexpected branch reached");
  }
  void visit(TensorErrorInst *inst) override {
    throw std::runtime_error("unexpected branch reached");
  }

  void visit(SetInst *inst) override {
    inst->getLval()->accept(*this);
    auto lval = (Symbol *)spilledItem;
    inst->getBase()->accept(*this);
    auto base = (Symbol *)spilledItem;
    inst->getOffset()->accept(*this);
    auto offset = (Symbol *)spilledItem;
    spilledInst = new SetInst(lval, base, offset, inst->getScalar());
  }

  void visit(LabelInst *inst) override { throw std::runtime_error("unexpected branch reached"); }
  void visit(GotoInst *inst) override { throw std::runtime_error("unexpected branch reached"); }

  void visit(CondJumpInst *inst) override {
    inst->getLval()->accept(*this);
    auto lval = (Value *)spilledItem;
    inst->getRval()->accept(*this);
    auto rval = (Value *)spilledItem;
    spilledInst = new CondJumpInst(inst->getOp(), lval, rval, inst->getLabel());
  }

  void doVisit(Instruction *I) {
    spilledInsts.push_back(I);
    auto GEN = result->getLivenessSets(I).getGEN(), KILL = result->getLivenessSets(I).getKILL();
    bool gened = GEN.find(spilledVar) != GEN.end(), killed = KILL.find(spilledVar) != KILL.end();

    // if the variable is not in GEN or KILL, means that it does not appear in this instruction
    if (!gened && !killed)
      return;

    newVar = nullptr;
    spilledInsts.pop_back();
    I->accept(*this);
    if (!newVar)
      throw std::runtime_error("unexpected branch reached");

    if (gened)
      spilledInsts.push_back(new AssignInst(newVar, memLoc));
    spilledInsts.push_back(spilledInst);
    if (killed)
      spilledInsts.push_back(new AssignInst(memLoc, newVar));
  }

  // setter that should be called for each function
  void loadSpillFuncInfo(FunctionToSpill *F, const FunctionLivenessResult *result) {
    this->result = result;
    spilledVar = F->getSpilledVar();
    spillPrefix = F->getSpillPrefix();
    spillCount = 0;
  }

private:
  // function wise info
  const Variable *spilledVar;
  std::string spillPrefix;
  const FunctionLivenessResult *result;
  int spillCount;

  // BB wise info
  std::vector<Instruction *> spilledInsts;

  // instruction wise buffer
  Variable *newVar;
  Item *spilledItem;
  Instruction *spilledInst;

  // global buffer
  MemoryLocation *memLoc =
      new MemoryLocation(Register::getRegister(Register::ID::RSP), new Number(0));

  static Spiller *instance;

  Spiller() = default;
  Spiller(const Spiller &) = delete;
  Spiller &operator=(const Spiller &) = delete;

  friend void spillInBB(BasicBlock *BB);
};

Spiller *Spiller::instance = nullptr;
auto spiller = Spiller::getInstance();

void spillInBB(BasicBlock *BB) {
  spiller->spilledInsts.clear();
  for (auto I : BB->getInstructions())
    spiller->doVisit(I);
  BB->instructions = spiller->spilledInsts;
}

void spillProgram(Program *P, const LivenessResult &result) {
  auto newMemLoc = new MemoryLocation(Register::getRegister(Register::ID::RSP), new Number(0));
  for (auto F : P->getFunctions()) {
    auto spilledF = (FunctionToSpill *)F;
    if (!spilledF->getSpilledVar())
      continue;

    spiller->loadSpillFuncInfo(spilledF, &result.getFunctionResult(spilledF));
    for (auto BB : F->getBasicBlocks())
      spillInBB(BB);
    spilledF->setSpilled(true);
  }
}

} // namespace L2