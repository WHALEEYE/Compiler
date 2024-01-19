#include <liveness_analyzer.h>

namespace L2 {
void livenessAnalyze();

void LivenessAnalyzer::visit(Register *reg) {}
void LivenessAnalyzer::visit(Variable *var) {}
void LivenessAnalyzer::visit(Number *num) {}
void LivenessAnalyzer::visit(CompareOp *op) {}
void LivenessAnalyzer::visit(ShiftOp *op) {}
void LivenessAnalyzer::visit(ArithOp *op) {}
void LivenessAnalyzer::visit(SelfModOp *op) {}
void LivenessAnalyzer::visit(MemoryLocation *mem) {}
void LivenessAnalyzer::visit(StackLocation *stack) {}
void LivenessAnalyzer::visit(FunctionName *name) {}
void LivenessAnalyzer::visit(Label *label) {}
void LivenessAnalyzer::visit(RetInst *inst) {}
void LivenessAnalyzer::visit(ShiftInst *inst) {}
void LivenessAnalyzer::visit(ArithInst *inst) {}
void LivenessAnalyzer::visit(SelfModInst *inst) {}
void LivenessAnalyzer::visit(AssignInst *inst) {}
void LivenessAnalyzer::visit(CompareAssignInst *inst) {}

std::vector<std::string> LivenessAnalyzer::getLiveOuts() { return {}; }
std::vector<std::string> LivenessAnalyzer::getLiveIns() { return {}; }

} // namespace L2