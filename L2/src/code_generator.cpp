#include <fstream>
#include <iostream>
#include <string>

#include <L2.h>
#include <code_generator.h>
#include <graph_colorer.h>
#include <spiller.h>

using namespace std;

namespace L2 {

class L1CodeGenerator : Visitor {
public:
  void visit(const Register *reg) override {
    buffer += reg->toStr();
    buffer += " ";
  }

  void visit(const Variable *var) override {
    auto reg = colorMap.at(var);
    buffer += Register::getRegister(reg)->toStr();
    buffer += " ";
  }

  void visit(const Number *num) override {
    buffer += num->toStr();
    buffer += " ";
  }

  void visit(const CompareOp *op) override {
    buffer += op->toStr();
    buffer += " ";
  }

  void visit(const ShiftOp *op) override {
    buffer += op->toStr();
    buffer += " ";
  }

  void visit(const ArithOp *op) override {
    buffer += op->toStr();
    buffer += " ";
  }

  void visit(const SelfModOp *op) override {
    buffer += op->toStr();
    buffer += " ";
  }

  void visit(const MemoryLocation *mem) override {
    buffer += "mem ";
    mem->getBase()->accept(*this);
    mem->getOffset()->accept(*this);
  }

  void visit(const StackLocation *stack) override {
    buffer += "mem rsp ";
    buffer += to_string(stack->getOffset()->getVal() + spillInfo->getSpillCount() * 8);
  }

  void visit(const FunctionName *name) override {
    buffer += name->toStr();
    buffer += " ";
  }

  void visit(const Label *label) override {
    buffer += label->toStr();
    buffer += " ";
  }

  void visit(const RetInst *I) override { buffer += "return"; }

  void visit(const ShiftInst *I) override {
    I->getLval()->accept(*this);
    I->getOp()->accept(*this);
    I->getRval()->accept(*this);
  }

  void visit(const ArithInst *I) override {
    I->getLval()->accept(*this);
    I->getOp()->accept(*this);
    I->getRval()->accept(*this);
  }

  void visit(const SelfModInst *I) override {
    I->getLval()->accept(*this);
    I->getOp()->accept(*this);
  }

  void visit(const AssignInst *I) override {
    I->getLval()->accept(*this);
    buffer += "<- ";
    I->getRval()->accept(*this);
  }

  void visit(const CompareAssignInst *I) override {
    I->getLval()->accept(*this);
    buffer += "<- ";
    I->getCmpLval()->accept(*this);
    I->getOp()->accept(*this);
    I->getCmpRval()->accept(*this);
  }

  void visit(const CallInst *I) override {
    buffer += "call ";
    I->getCallee()->accept(*this);
    I->getArgNum()->accept(*this);
  }

  void visit(const PrintInst *I) override { buffer += I->toStr(); }

  void visit(const InputInst *I) override { buffer += I->toStr(); }

  void visit(const AllocateInst *I) override { buffer += I->toStr(); }

  void visit(const TupleErrorInst *I) override { buffer += I->toStr(); }

  void visit(const TensorErrorInst *I) override { buffer += I->toStr(); }

  void visit(const SetInst *I) override {
    I->getLval()->accept(*this);
    buffer += "@ ";
    I->getBase()->accept(*this);
    I->getOffset()->accept(*this);
    I->getScalar()->accept(*this);
  }

  void visit(const LabelInst *I) override { buffer += I->toStr(); }

  void visit(const GotoInst *I) override { buffer += I->toStr(); }

  void visit(const CondJumpInst *I) override {
    buffer += "cjump ";
    I->getLval()->accept(*this);
    I->getOp()->accept(*this);
    I->getRval()->accept(*this);
    I->getLabel()->accept(*this);
  }

  void doVisit(const Instruction *I) {
    buffer = "";
    I->accept(*this);
    instructions.push_back(buffer);
  }

  static L1CodeGenerator &getInstance() {
    if (instance == nullptr)
      instance = new L1CodeGenerator();
    return *instance;
  }

  const vector<string> &getInstructions() const { return instructions; }

  void loadFunctionInfo(const ColorResult &result) {
    this->colorMap = result.getColorMap();
    this->spillInfo = &result.getSpillInfo();
    this->instructions.clear();
  }

private:
  void printWithIndent(const string &str) { cout << indent << str << endl; }

  string buffer;
  vector<string> instructions;
  const string indent = "    ";

  // info used for generating code
  ColorMap colorMap;
  const SpillInfo *spillInfo;

  // singleton
  L1CodeGenerator() = default;
  L1CodeGenerator(const L1CodeGenerator &) = delete;
  L1CodeGenerator &operator=(const L1CodeGenerator &) = delete;
  static L1CodeGenerator *instance;
};

L1CodeGenerator *L1CodeGenerator::instance = nullptr;

void generate_code(Program *P, unordered_map<const Function *, const ColorResult *> &results) {

  /*
   * Open the output file.
   */
  ofstream outputFile;
  outputFile.open("prog.L1");

  auto &generator = L1CodeGenerator::getInstance();

  outputFile << "(" << P->getEntryPointLabel() << endl;
  for (auto F : P->getFunctions()) {
    auto colorResult = results.at(F);
    generator.loadFunctionInfo(*colorResult);
    outputFile << "  (" << F->getName() << "\n    " << F->getParamNum() << " "
               << colorResult->getSpillInfo().getSpillCount() << "\n";

    for (auto BB : F->getBasicBlocks())
      for (auto I : BB->getInstructions())
        generator.doVisit(I);

    for (auto I : generator.getInstructions())
      outputFile << "    " << I << endl;

    outputFile << "  )" << std::endl;
  }
  outputFile << ")" << endl;
  return;
}
} // namespace L2