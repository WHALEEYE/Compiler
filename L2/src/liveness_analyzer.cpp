#include <iostream>
#include <map>
#include <queue>
#include <unordered_set>

#include <L2.h>
#include <liveness_analyzer.h>

namespace L2 {

class GenKillCalculator : public Visitor {
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
    now = &KILL;
    inst->getLval()->accept(*this);
    now = &GEN;
    inst->getLval()->accept(*this);
    inst->getRval()->accept(*this);
  }

  void visit(ArithInst *inst) {
    now = &KILL;
    inst->getLval()->accept(*this);
    now = &GEN;
    inst->getLval()->accept(*this);
    inst->getRval()->accept(*this);
  }

  void visit(SelfModInst *inst) {
    now = &KILL;
    inst->getLval()->accept(*this);
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

  static GenKillCalculator *getInstance() {
    if (instance == nullptr)
      instance = new GenKillCalculator();
    return instance;
  }

  void doVisit(Instruction *I) {
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
  const std::unordered_set<Register *> &callerSaved = Register::getCallerSavedRegisters(),
                                       &calleeSaved = Register::getCalleeSavedRegisters();
  const std::vector<Register *> &args = Register::getArgRegisters();

  void handleCall(int64_t argNum) {
    KILL.insert(callerSaved.begin(), callerSaved.end());
    for (int i = 0; i < std::min(argNum, (int64_t)6); i++)
      GEN.insert(args[i]);
  }
};

void calculateGenKill(Function *F, FunctionLivenessResult &functionResult) {
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

void FunctionLivenessResult::dump() const {
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

const LivenessSets &FunctionLivenessResult::getLivenessSets(Instruction *I) const {
  return result.at(I);
}

bool setEqual(const std::unordered_set<const Symbol *> &a, const std::unordered_set<const Symbol *> &b) {
  if (a.size() != b.size())
    return false;

  for (auto var : a)
    if (b.find(var) == b.end())
      return false;

  return true;
}

bool analyzeInBB(BasicBlock *BB, FunctionLivenessResult &functionResult, bool visited) {
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

const FunctionLivenessResult &LivenessResult::getFunctionResult(const Function *F) const {
  return functionResults.at(F);
}

const LivenessResult &analyzeLiveness(const Program *P) {
  auto livenessResult = new LivenessResult();
  for (auto F : P->getFunctions()) {
    auto &functionResult = livenessResult->functionResults[F];

    // first, initialize the instBuffer
    for (auto BB : F->getBasicBlocks())
      for (auto I : BB->getInstructions())
        functionResult.instBuffer.push_back(I);

    calculateGenKill(F, functionResult);

    std::queue<BasicBlock *> workq;
    std::map<BasicBlock *, bool> visited;
    for (auto pB = F->getBasicBlocks().rbegin(); pB != F->getBasicBlocks().rend(); pB++)
      workq.push(*pB);

    while (!workq.empty()) {
      auto BB = workq.front();
      workq.pop();

      if (analyzeInBB(BB, functionResult, visited[BB]))
        for (auto pred : BB->getPredecessors())
          workq.push(pred);

      visited[BB] = true;
    }
  }

  return *livenessResult;
}

} // namespace L2