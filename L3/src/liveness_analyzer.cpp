#include <iostream>
#include <map>
#include <queue>
#include <unordered_set>

#include <helper.h>
#include <liveness_analyzer.h>

namespace L3 {

class GenKillCalculator : public Visitor {
public:
  void visit(const Variable *var) override {
    debug("visiting variable " + var->toStr());
    now->insert(var);
  }

  void visit(const Number *num) override {}
  void visit(const CompareOp *op) override {}
  void visit(const ArithOp *op) override {}
  void visit(const FunctionName *name) override {}
  void visit(const Label *label) override {}
  void visit(const Arguments *args) override {
    for (auto arg : args->getArgs())
      arg->accept(*this);
  }
  void visit(const Parameters *params) override {}
  void visit(const RuntimeFunction *func) override {}

  void visit(const AssignInst *inst) override {
    now = &KILL;
    inst->getLhs()->accept(*this);
    now = &GEN;
    inst->getRhs()->accept(*this);
  }

  void visit(const ArithInst *inst) override {
    now = &KILL;
    inst->getRst()->accept(*this);
    now = &GEN;
    inst->getLhs()->accept(*this);
    inst->getRhs()->accept(*this);
  }

  void visit(const CompareInst *inst) override {
    now = &KILL;
    inst->getRst()->accept(*this);
    now = &GEN;
    inst->getLhs()->accept(*this);
    inst->getRhs()->accept(*this);
  }

  void visit(const LoadInst *inst) override {
    now = &KILL;
    inst->getVal()->accept(*this);
    now = &GEN;
    inst->getAddr()->accept(*this);
  }

  void visit(const StoreInst *inst) override {
    now = &GEN;
    inst->getAddr()->accept(*this);
    inst->getVal()->accept(*this);
  }

  void visit(const RetInst *inst) override {}
  void visit(const RetValueInst *inst) override {
    now = &GEN;
    inst->getVal()->accept(*this);
  }

  void visit(const LabelInst *inst) override {}
  void visit(const BranchInst *inst) override {}
  void visit(const CondBranchInst *inst) override {}

  void visit(const CallInst *inst) override {
    now = &GEN;
    inst->getCallee()->accept(*this);
    inst->getArgs()->accept(*this);
  }

  void visit(const CallAssignInst *inst) override {
    now = &KILL;
    inst->getRst()->accept(*this);
    now = &GEN;
    inst->getCallee()->accept(*this);
    inst->getArgs()->accept(*this);
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

  std::unordered_set<const Variable *> &getGEN() { return GEN; }
  std::unordered_set<const Variable *> &getKILL() { return KILL; }

private:
  std::unordered_set<const Variable *> GEN, KILL, *now{};

  GenKillCalculator(){};
  static GenKillCalculator *instance;
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

const std::unordered_set<const Variable *> &LivenessSets::getGEN() const { return GEN; }
const std::unordered_set<const Variable *> &LivenessSets::getKILL() const { return KILL; }
const std::unordered_set<const Variable *> &LivenessSets::getIN() const { return IN; }
const std::unordered_set<const Variable *> &LivenessSets::getOUT() const { return OUT; }

void LivenessResult::dump() const {
  std::cout << "(" << std::endl << "(in" << std::endl;
  for (auto I : instBuffer) {
    auto &IN = result.at(I).getIN();
    std::cout << "(";

    for (auto Variable : IN)
      std::cout << Variable->toStr() << " ";

    std::cout << ")" << std::endl;
  }

  std::cout << ")" << std::endl << std::endl << "(out" << std::endl;

  for (auto I : instBuffer) {
    auto &OUT = result.at(I).getOUT();
    std::cout << "(";

    for (auto Variable : OUT)
      std::cout << Variable->toStr() << " ";

    std::cout << ")" << std::endl;
  }

  std::cout << ")" << std::endl << std::endl << ")" << std::endl;
}

const LivenessSets &LivenessResult::getLivenessSets(const Instruction *I) const { return result.at(I); }

bool setEqual(const std::unordered_set<const Variable *> &a, const std::unordered_set<const Variable *> &b) {
  if (a.size() != b.size())
    return false;

  for (auto var : a)
    if (b.find(var) == b.end())
      return false;

  return true;
}

bool analyzeInBB(const BasicBlock *BB, LivenessResult &functionResult, bool visited) {
  std::unordered_set<const Variable *> buffer;
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

} // namespace L3