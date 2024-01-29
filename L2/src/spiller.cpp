#include "liveness_analyzer.h"
#include <L2.h>
#include <spiller.h>
#include <stdexcept>

namespace L2 {
FunctionToSpill::FunctionToSpill(std::string name) : Function(name) { spilled = false; }

bool FunctionToSpill::getSpilled() const { return spilled; }
void FunctionToSpill::setSpilled(bool spilled) { this->spilled = spilled; }
const Variable *FunctionToSpill::getSpilledVar() const { return spilledVar; }
void FunctionToSpill::setSpilledVar(const Variable *spilledVar) { this->spilledVar = spilledVar; }
std::string FunctionToSpill::getSpillPrefix() const { return spillPrefix; }
void FunctionToSpill::setSpillPrefix(std::string spillPrefix) { this->spillPrefix = spillPrefix; }

bool FunctionSpillInfo::isSpilled(const Variable *var) const {
  return varSpillInfos.find(var) != varSpillInfos.end();
}

VarSpillInfo *FunctionSpillInfo::getVarSpillInfo(const Variable *var) {
  if (!isSpilled(var)) {
    varSpillInfos[var].memLoc =
        new MemoryLocation(Register::getRegister(Register::ID::RBP), new Number(8 * spillCount));
    spillCount++;
  }
  return &varSpillInfos[var];
}

std::unordered_set<const Variable *> getIntersect(const std::unordered_set<const Variable *> &set1,
                                                  const std::unordered_set<const Symbol *> &set2) {
  std::unordered_set<const Variable *> result;
  for (auto var : set1)
    if (set2.find(var) != set2.end())
      result.insert(var);

  return result;
}

class Spiller : Visitor {
public:
  static Spiller *getInstance() {
    if (!instance)
      instance = new Spiller();
    return instance;
  }

  // when a item is visited, spill every item possible that equals to spilledVar
  // then store the updated version in buffer
  void visit(const Register *reg) override { spilledItem = reg; }

  void visit(const Variable *var) override {
    if (varsToBeSpilled.find(var) == varsToBeSpilled.end()) {
      spilledItem = var;
      return;
    }
    auto varSpillInfo = spillInfo->getVarSpillInfo(var);
    if (!varSpillInfo->newVar)
      varSpillInfo->newVar =
          new Variable(var->getName() + std::to_string(varSpillInfo->nextPostfix++));
    spilledItem = varSpillInfo->newVar;
  }

  void visit(const Number *num) override { spilledItem = num; }
  void visit(const CompareOp *op) override {
    throw std::runtime_error("unexpected branch reached");
  }
  void visit(const ShiftOp *op) override { throw std::runtime_error("unexpected branch reached"); }
  void visit(const ArithOp *op) override { throw std::runtime_error("unexpected branch reached"); }
  void visit(const SelfModOp *op) override {
    throw std::runtime_error("unexpected branch reached");
  }

  void visit(const MemoryLocation *mem) override {
    mem->getBase()->accept(*this);
    auto base = (Symbol *)spilledItem;
    spilledItem = new MemoryLocation(base, mem->getOffset());
  }

  void visit(const StackLocation *stack) override {
    throw std::runtime_error("unexpected branch reached");
  }

  void visit(const FunctionName *name) override { spilledItem = name; }
  void visit(const Label *label) override { spilledItem = label; }

  // when a instruction is visited, spill every item possible that equals to spilledVar
  // and store the updated version in spilledInst
  void visit(const RetInst *inst) override {
    throw std::runtime_error("unexpected branch reached");
  }

  void visit(const ShiftInst *inst) override {
    inst->getLval()->accept(*this);
    auto lval = (Symbol *)spilledItem;
    inst->getRval()->accept(*this);
    auto rval = (Value *)spilledItem;
    spilledInst = new ShiftInst(inst->getOp(), lval, rval);
  }

  void visit(const ArithInst *inst) override {
    inst->getLval()->accept(*this);
    auto lval = (Symbol *)spilledItem;
    inst->getRval()->accept(*this);
    auto rval = (Value *)spilledItem;
    spilledInst = new ArithInst(inst->getOp(), lval, rval);
  }

  void visit(const SelfModInst *inst) override {
    inst->getLval()->accept(*this);
    auto lval = (Symbol *)spilledItem;
    spilledInst = new SelfModInst(inst->getOp(), lval);
  }

  void visit(const AssignInst *inst) override {
    inst->getLval()->accept(*this);
    auto lval = (Symbol *)spilledItem;
    inst->getRval()->accept(*this);
    auto rval = (Value *)spilledItem;
    spilledInst = new AssignInst(lval, rval);
  }

  void visit(const CompareAssignInst *inst) override {
    inst->getLval()->accept(*this);
    auto lval = (Symbol *)spilledItem;
    inst->getCmpLval()->accept(*this);
    auto cmpLval = (Value *)spilledItem;
    inst->getCmpRval()->accept(*this);
    auto cmpRval = (Value *)spilledItem;
    spilledInst = new CompareAssignInst(lval, inst->getOp(), cmpLval, cmpRval);
  }

  void visit(const CallInst *inst) override {
    inst->getCallee()->accept(*this);
    auto callee = (Item *)spilledItem;
    spilledInst = new CallInst(callee, inst->getArgNum());
  }

  void visit(const PrintInst *inst) override {
    throw std::runtime_error("unexpected branch reached");
  }
  void visit(const InputInst *inst) override {
    throw std::runtime_error("unexpected branch reached");
  }
  void visit(const AllocateInst *inst) override {
    throw std::runtime_error("unexpected branch reached");
  }
  void visit(const TupleErrorInst *inst) override {
    throw std::runtime_error("unexpected branch reached");
  }
  void visit(const TensorErrorInst *inst) override {
    throw std::runtime_error("unexpected branch reached");
  }

  void visit(const SetInst *inst) override {
    inst->getLval()->accept(*this);
    auto lval = (Symbol *)spilledItem;
    inst->getBase()->accept(*this);
    auto base = (Symbol *)spilledItem;
    inst->getOffset()->accept(*this);
    auto offset = (Symbol *)spilledItem;
    spilledInst = new SetInst(lval, base, offset, inst->getScalar());
  }

  void visit(const LabelInst *inst) override {
    throw std::runtime_error("unexpected branch reached");
  }
  void visit(const GotoInst *inst) override {
    throw std::runtime_error("unexpected branch reached");
  }

  void visit(const CondJumpInst *inst) override {
    inst->getLval()->accept(*this);
    auto lval = (Value *)spilledItem;
    inst->getRval()->accept(*this);
    auto rval = (Value *)spilledItem;
    spilledInst = new CondJumpInst(inst->getOp(), lval, rval, inst->getLabel());
  }

  void doVisit(const Instruction *I) {
    spilledInsts.push_back(I);
    auto GEN = result->getLivenessSets(I).getGEN(), KILL = result->getLivenessSets(I).getKILL();
    auto gened = getIntersect(varsToBeSpilled, GEN), killed = getIntersect(varsToBeSpilled, KILL);

    // if the variable is not in GEN or KILL, means that it does not appear in this instruction
    if (gened.empty() && killed.empty())
      return;

    for (auto var : varsToBeSpilled)
      spillInfo->getVarSpillInfo(var)->newVar = nullptr;

    spilledInsts.pop_back();
    I->accept(*this);

    for (auto var : gened) {
      auto varSpillInfo = spillInfo->getVarSpillInfo(var);
      spilledInsts.push_back(new AssignInst(varSpillInfo->newVar, varSpillInfo->memLoc));
    }
    spilledInsts.push_back(spilledInst);
    for (auto var : killed) {
      auto varSpillInfo = spillInfo->getVarSpillInfo(var);
      spilledInsts.push_back(new AssignInst(varSpillInfo->memLoc, varSpillInfo->newVar));
    }
  }

  // setter that should be called for each function
  void loadSpillInfo(FunctionSpillInfo *spillInfo, const FunctionLivenessResult *result,
                     const std::unordered_set<const Variable *> &varsToBeSpilled) {
    this->result = result;
    this->spillInfo = spillInfo;
    this->varsToBeSpilled = varsToBeSpilled;
  }

private:
  // function wise info
  const FunctionLivenessResult *result;
  FunctionSpillInfo *spillInfo;
  std::unordered_set<const Variable *> varsToBeSpilled;

  // BB wise info
  std::vector<const Instruction *> spilledInsts;

  // instruction wise buffer
  const Item *spilledItem;
  const Instruction *spilledInst;

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
  for (auto F : P->getFunctions()) {
    auto spilledF = (FunctionToSpill *)F;
    if (!spilledF->getSpilledVar())
      continue;
    auto functionSpillInfo = new FunctionSpillInfo();
    auto varsToBeSpilled = std::unordered_set<const Variable *>{spilledF->getSpilledVar()};
    spiller->loadSpillInfo(functionSpillInfo, &result.getFunctionResult(F), varsToBeSpilled);
    for (auto BB : F->getBasicBlocks())
      spillInBB(BB);
    spilledF->setSpilled(true);
  }
}

void spillFunction(Function *F, const FunctionLivenessResult &livenessResult,
                                 const std::unordered_set<const Variable *> &varsToBeSpilled,
                                 std::string spillPrefix, FunctionSpillInfo &functionSpillInfo) {
  spiller->loadSpillInfo(&functionSpillInfo, &livenessResult, varsToBeSpilled);
  for (auto BB : F->getBasicBlocks())
    spillInBB(BB);
}

} // namespace L2