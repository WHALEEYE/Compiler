#pragma once

#include <L2.h>

namespace L2 {

void livenessAnalyze();

class LivenessAnalyzer : public L2Visitor {
public:
  void visit(Register *reg) override;
  void visit(Variable *var) override;
  void visit(Number *num) override;
  void visit(CompareOp *op) override;
  void visit(ShiftOp *op) override;
  void visit(ArithOp *op) override;
  void visit(SelfModOp *op) override;
  void visit(MemoryLocation *mem) override;
  void visit(StackLocation *stack) override;
  void visit(FunctionName *name) override;
  void visit(Label *label) override;
  void visit(RetInst *inst) override;
  void visit(ShiftInst *inst) override;
  void visit(ArithInst *inst) override;
  void visit(SelfModInst *inst) override;
  void visit(AssignInst *inst) override;
  void visit(CompareAssignInst *inst) override;

  std::vector<std::string> getLiveOuts();
  std::vector<std::string> getLiveIns();
};
} // namespace L2