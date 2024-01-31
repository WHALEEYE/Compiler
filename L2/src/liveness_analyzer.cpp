#include <iostream>
#include <map>
#include <queue>
#include <unordered_set>

#include <L2.h>
#include <liveness_analyzer.h>

namespace L2 {

class GenKillCalculator : public Visitor {
public:
  void visit(const Register *reg) override {
    debug("visiting register " + reg->toStr());
    if (reg->getID() == Register::ID::RSP)
      return;
    now->insert(reg);
  }

  void visit(const Variable *var) override {
    debug("visiting variable " + var->toStr());
    now->insert(var);
  }

  void visit(const Number *num) override {}
  void visit(const CompareOp *op) override {}
  void visit(const ShiftOp *op) override {}
  void visit(const ArithOp *op) override {}
  void visit(const SelfModOp *op) override {}

  void visit(const MemoryLocation *mem) override {
    // no matter how mem loc is accessed, the base register is always brought alive
    auto original = now;
    now = &GEN;
    mem->getBase()->accept(*this);
    now = original;
  }
  void visit(const StackLocation *stack) override {}
  void visit(const FunctionName *name) override {}
  void visit(const Label *label) override {}

  void visit(const RetInst *inst) override {
    GEN.insert(Register::getRegister(Register::ID::RAX));
    auto &calleeSaved = Register::getCalleeSavedRegisters();
    GEN.insert(calleeSaved.begin(), calleeSaved.end());
  }

  void visit(const ShiftInst *inst) override {
    now = &KILL;
    inst->getLval()->accept(*this);
    now = &GEN;
    inst->getLval()->accept(*this);
    inst->getRval()->accept(*this);
  }

  void visit(const ArithInst *inst) override {
    now = &KILL;
    inst->getLval()->accept(*this);
    now = &GEN;
    inst->getLval()->accept(*this);
    inst->getRval()->accept(*this);
  }

  void visit(const SelfModInst *inst) override {
    now = &KILL;
    inst->getLval()->accept(*this);
    now = &GEN;
    inst->getLval()->accept(*this);
  }

  void visit(const AssignInst *inst) override {
    now = &KILL;
    inst->getLval()->accept(*this);
    now = &GEN;
    inst->getRval()->accept(*this);
  }

  void visit(const CompareAssignInst *inst) override {
    now = &KILL;
    inst->getLval()->accept(*this);
    now = &GEN;
    inst->getCmpLval()->accept(*this);
    inst->getCmpRval()->accept(*this);
  }

  void visit(const CallInst *inst) override {
    handleCall(inst->getArgNum()->getVal());
    now = &GEN;
    inst->getCallee()->accept(*this);
  }

  void visit(const PrintInst *inst) override { handleCall(1); }

  void visit(const InputInst *inst) override { handleCall(0); }

  void visit(const AllocateInst *inst) override { handleCall(2); }

  void visit(const TupleErrorInst *inst) override { handleCall(3); }

  void visit(const TensorErrorInst *inst) override { handleCall(inst->getArgNum()->getVal()); }

  void visit(const SetInst *inst) override {
    now = &KILL;
    inst->getLval()->accept(*this);
    now = &GEN;
    inst->getBase()->accept(*this);
    inst->getOffset()->accept(*this);
  }
  void visit(const LabelInst *inst) override {}

  void visit(const GotoInst *inst) override {}

  void visit(const CondJumpInst *inst) override {
    now = &GEN;
    inst->getLval()->accept(*this);
    inst->getRval()->accept(*this);
  }

  static GenKillCalculator *getInstance() {
    if (instance == nullptr)
      instance = new GenKillCalculator();
    return instance;
  }

  void doVisit(const Instruction *I) {
    GEN.clear();
    KILL.clear();
    I->accept(*this);
  }

  std::unordered_set<const Symbol *> &getGEN() { return GEN; }
  std::unordered_set<const Symbol *> &getKILL() { return KILL; }

private:
  std::unordered_set<const Symbol *> GEN, KILL, *now;

  GenKillCalculator(){};
  static GenKillCalculator *instance;

  // used as a readonly buffer
  const std::unordered_set<const Register *> &callerSaved = Register::getCallerSavedRegisters(),
                                             &calleeSaved = Register::getCalleeSavedRegisters();
  const std::vector<const Register *> &args = Register::getArgRegisters();

  void handleCall(int64_t argNum) {
    KILL.insert(callerSaved.begin(), callerSaved.end());
    for (int i = 0; i < std::min(argNum, (int64_t)6); i++)
      GEN.insert(args[i]);
  }
};

void calculateGenKill(const Function *F, LivenessResult &functionResult) {
  auto &result = functionResult.result;
  auto calculator = GenKillCalculator::getInstance();
  for (auto I : functionResult.instBuffer) {
    calculator->doVisit(I);
    result[I].GEN = calculator->getGEN();
    result[I].KILL = calculator->getKILL();
  }
}

GenKillCalculator *GenKillCalculator::instance = nullptr;

const std::unordered_set<const Symbol *> &LivenessSets::getGEN() const { return GEN; }
const std::unordered_set<const Symbol *> &LivenessSets::getKILL() const { return KILL; }
const std::unordered_set<const Symbol *> &LivenessSets::getIN() const { return IN; }
const std::unordered_set<const Symbol *> &LivenessSets::getOUT() const { return OUT; }

void LivenessResult::dump() const {
  std::cout << "(" << std::endl << "(in" << std::endl;
  for (auto I : instBuffer) {
    auto &IN = result.at(I).getIN();
    std::cout << "(";

    for (auto symbol : IN)
      std::cout << symbol->toStr() << " ";

    std::cout << ")" << std::endl;
  }

  std::cout << ")" << std::endl << std::endl << "(out" << std::endl;

  for (auto I : instBuffer) {
    auto &OUT = result.at(I).getOUT();
    std::cout << "(";

    for (auto symbol : OUT)
      std::cout << symbol->toStr() << " ";

    std::cout << ")" << std::endl;
  }

  std::cout << ")" << std::endl << std::endl << ")" << std::endl;
}

const LivenessSets &LivenessResult::getLivenessSets(const Instruction *I) const {
  return result.at(I);
}

bool setEqual(const std::unordered_set<const Symbol *> &a,
              const std::unordered_set<const Symbol *> &b) {
  if (a.size() != b.size())
    return false;

  for (auto var : a)
    if (b.find(var) == b.end())
      return false;

  return true;
}

bool analyzeInBB(const BasicBlock *BB, LivenessResult &functionResult, bool visited) {
  std::unordered_set<const Symbol *> buffer;
  auto &result = functionResult.result;
  for (auto succ : BB->getSuccessors()) {
    auto &succIN = result[succ->getFirstInstruction()].IN;
    buffer.insert(succIN.begin(), succIN.end());
  }

  if (visited && setEqual(buffer, result[BB->getTerminator()].OUT))
    return false;

  for (auto pI = BB->getInstructions().rbegin(); pI != BB->getInstructions().rend(); pI++) {
    auto I = *pI;
    result[I].OUT = buffer;
    auto &GEN = result[I].GEN, &KILL = result[I].KILL;
    for (auto var : KILL)
      buffer.erase(var);
    buffer.insert(GEN.begin(), GEN.end());
    result[I].IN = buffer;
  }
  return true;
}

const LivenessResult &analyzeLiveness(const Function *F) {
  auto livenessResult = new LivenessResult();

  // first, initialize the instBuffer
  for (auto BB : F->getBasicBlocks())
    for (auto I : BB->getInstructions())
      livenessResult->instBuffer.push_back(I);

  calculateGenKill(F, *livenessResult);

  std::queue<BasicBlock *> workq;
  std::map<BasicBlock *, bool> visited;
  for (auto pB = F->getBasicBlocks().rbegin(); pB != F->getBasicBlocks().rend(); pB++)
    workq.push(*pB);

  while (!workq.empty()) {
    auto BB = workq.front();
    workq.pop();

    if (analyzeInBB(BB, *livenessResult, visited[BB]))
      for (auto pred : BB->getPredecessors())
        workq.push(pred);

    visited[BB] = true;
  }

  return *livenessResult;
}

} // namespace L2