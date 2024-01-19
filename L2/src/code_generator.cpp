#include <L2.h>
#include <code_generator.h>
#include <fstream>

using namespace std;

namespace L2 {

class CodeGenerator : public L2::L2Visitor {
  void visit(Register *reg) override {
    // TODO
  }
  void visit(Variable *var) override {
    // TODO
  }
  void visit(Number *num) override {
    // TODO
  }
  void visit(CompareOp *op) override {
    // TODO
  }
  void visit(ShiftOp *op) override {
    // TODO
  }
  void visit(ArithOp *op) override {
    // TODO
  }
  void visit(SelfModOp *op) override {
    // TODO
  }
  void visit(MemoryLocation *mem) override {
    // TODO
  }
  void visit(StackLocation *stack) override {
    // TODO
  }
  void visit(FunctionName *name) override {
    // TODO
  }
  void visit(Label *label) override {
    // TODO
  }
  void visit(RetInst *inst) override {
    // TODO
  }
  void visit(ShiftInst *inst) override {
    // TODO
  }
  void visit(ArithInst *inst) override {
    // TODO
  }
  void visit(SelfModInst *inst) override {
    // TODO
  }
  void visit(AssignInst *inst) override {
    // TODO
  }
  void visit(CompareAssignInst *inst) override {
    // TODO
  }
  void visit(CallInst *inst) override {
    // TODO
  }
  void visit(PrintInst *inst) override {
    // TODO
  }
  void visit(InputInst *inst) override {
    // TODO
  }
  void visit(AllocateInst *inst) override {
    // TODO
  }
  void visit(TupleErrorInst *inst) override {
    // TODO
  }
  void visit(TensorErrorInst *inst) override {
    // TODO
  }
  void visit(SetInst *inst) override {
    // TODO
  }
  void visit(LabelInst *inst) override {
    // TODO
  }
  void visit(GotoInst *inst) override {
    // TODO
  }
  void visit(CondJumpInst *inst) override {
    // TODO
  }
  void visit(Function *func) override {
    // TODO
  }
  void visit(Program *prog) override {
    // TODO
  }
};

void generate_code(Program P) {

  /*
   * Open the output file.
   */
  std::ofstream outputFile;
  outputFile.open("prog.S");
  return;
}
} // namespace L2
