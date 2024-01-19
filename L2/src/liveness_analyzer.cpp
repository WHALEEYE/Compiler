#include <L2.h>
#include <cstdio>
#include <iostream>
#include <liveness_analyzer.h>
#include <map>
#include <queue>
#include <unordered_set>

namespace L2 {
typedef std::unordered_set<std::string> LivenessSet;

bool setEqual(LivenessSet &a, LivenessSet &b) {
  if (a.size() != b.size())
    return false;

  for (auto var : a)
    if (b.find(var) == b.end())
      return false;

  return true;
}

class LivenessInfo {
public:
  LivenessSet IN, OUT;
};

class LivenessAnalyzer : public Visitor {
public:
  void visit(Register *reg) {
    debug("visiting register " + reg->toStr());
    if (deleting)
      buffer.erase(reg->toStr());
    else if (reg->getID() != RegisterID::RSP)
      buffer.insert(reg->toStr());
  }
  void visit(Variable *var) {
    debug("visiting variable " + var->toStr());
    if (deleting)
      buffer.erase(var->toStr());
    else
      buffer.insert(var->toStr());
  }
  void visit(Number *num) {}
  void visit(CompareOp *op) {}
  void visit(ShiftOp *op) {}
  void visit(ArithOp *op) {}
  void visit(SelfModOp *op) {}
  void visit(MemoryLocation *mem) {
    // no matter how mem loc is accessed, the base register is always brought alive
    bool oldDeleting = deleting;
    deleting = false;
    mem->getBase()->accept(*this);
    deleting = oldDeleting;
  }
  void visit(StackLocation *stack) {}
  void visit(FunctionName *name) {}
  void visit(Label *label) {}
  void visit(RetInst *inst) {
    buffer.insert("rax");
    auto calleeSaved = Register::getCalleeSavedRegisters();
    buffer.insert(calleeSaved.begin(), calleeSaved.end());
  }

  void visit(ShiftInst *inst) {
    deleting = true;
    inst->getLval()->accept(*this);
    deleting = false;
    inst->getRval()->accept(*this);
  }

  void visit(ArithInst *inst) {
    // no need to delete lval, gonna be inserted anyway
    deleting = false;
    inst->getLval()->accept(*this);
    inst->getRval()->accept(*this);
  }

  void visit(SelfModInst *inst) {
    deleting = false;
    inst->getLval()->accept(*this);
  }

  void visit(AssignInst *inst) {
    deleting = true;
    inst->getLval()->accept(*this);
    deleting = false;
    inst->getRval()->accept(*this);
  }

  void visit(CompareAssignInst *inst) {
    deleting = true;
    inst->getLval()->accept(*this);
    deleting = false;
    inst->getCmpLval()->accept(*this);
    inst->getCmpRval()->accept(*this);
  }

  void visit(CallInst *inst) {
    auto callerSaved = Register::getCallerSavedRegisters();
    for (auto reg : callerSaved)
      buffer.erase(reg);
    deleting = false;

    inst->getCallee()->accept(*this);
    auto argNum = inst->getArgNum()->getVal();

    auto args = Register::getArgRegisters(argNum);
    buffer.insert(args.begin(), args.end());
  }

  void visit(PrintInst *inst) {
    auto callerSaved = Register::getCallerSavedRegisters();
    for (auto reg : callerSaved)
      buffer.erase(reg);
    buffer.insert("rdi");
  }

  void visit(InputInst *inst) {
    auto callerSaved = Register::getCallerSavedRegisters();
    for (auto reg : callerSaved)
      buffer.erase(reg);
  }

  void visit(AllocateInst *inst) {
    auto callerSaved = Register::getCallerSavedRegisters();
    for (auto reg : callerSaved)
      buffer.erase(reg);
    buffer.insert("rdi");
    buffer.insert("rsi");
  }

  void visit(TupleErrorInst *inst) {
    auto callerSaved = Register::getCallerSavedRegisters();
    for (auto reg : callerSaved)
      buffer.erase(reg);
  }

  void visit(TensorErrorInst *inst) {
    auto callerSaved = Register::getCallerSavedRegisters();
    for (auto reg : callerSaved)
      buffer.erase(reg);
  }

  void visit(SetInst *inst) {
    deleting = true;
    inst->getLval()->accept(*this);
    deleting = false;
    inst->getBase()->accept(*this);
    inst->getOffset()->accept(*this);
  }
  void visit(LabelInst *inst) {}

  void visit(GotoInst *inst) {}

  void visit(CondJumpInst *inst) {
    deleting = false;
    inst->getLval()->accept(*this);
    inst->getRval()->accept(*this);
  }

  void setBuffer(LivenessSet &buffer) { this->buffer = buffer; }
  LivenessSet &getBuffer() { return buffer; }

private:
  LivenessSet buffer;
  bool deleting = false;
};

std::map<Instruction *, LivenessInfo> info;
LivenessAnalyzer analyzer;

void printResult(Function *F) {
  std::cout << "(" << std::endl << "(in" << std::endl;
  for (auto BB : F->getBasicBlocks())
    for (auto I : BB->getInstructions()) {
      std::cout << "(";
      for (auto pV = info[I].IN.begin(); pV != info[I].IN.end();) {
        std::cout << *pV;
        if (++pV != info[I].IN.end())
          std::cout << " ";
      }
      std::cout << ")" << std::endl;
    }

  std::cout << ")" << std::endl << std::endl << "(out" << std::endl;

  for (auto BB : F->getBasicBlocks())
    for (auto I : BB->getInstructions()) {
      std::cout << "(";
      for (auto pV = info[I].OUT.begin(); pV != info[I].OUT.end();) {
        std::cout << *pV;
        if (++pV != info[I].OUT.end())
          std::cout << " ";
      }
      std::cout << ")" << std::endl;
    }

  std::cout << ")" << std::endl << std::endl << ")" << std::endl;
}

bool analyzeInBB(BasicBlock *BB) {
  // if the last instruction of the BB is not in the map, then it's not analyzed before
  bool first = info.find(BB->getTerminator()) == info.end();
  LivenessSet buffer;
  for (auto succ : BB->getSuccessors()) {
    auto succInfo = info[succ->getFirstInstruction()];
    buffer.insert(succInfo.IN.begin(), succInfo.IN.end());
  }
  if (!first && setEqual(buffer, info[BB->getTerminator()].OUT))
    return false;

  analyzer.setBuffer(buffer);
  for (auto pI = BB->getInstructions().rbegin(); pI != BB->getInstructions().rend(); pI++) {
    auto I = *pI;
    info[I].OUT = analyzer.getBuffer();
    I->accept(analyzer);
    info[I].IN = analyzer.getBuffer();
  }
  return true;
}

void livenessAnalyze(Function *F) {
  std::queue<BasicBlock *> workq;
  for (auto pB = F->getBasicBlocks().rbegin(); pB != F->getBasicBlocks().rend(); pB++)
    workq.push(*pB);

  while (!workq.empty()) {
    auto BB = workq.front();
    workq.pop();
    if (analyzeInBB(BB)) {
      for (auto pred : BB->getPredecessors())
        workq.push(pred);
    }
  }

  printResult(F);
}

} // namespace L2