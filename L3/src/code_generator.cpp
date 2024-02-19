#include "tree.h"
#include <algorithm>
#include <fstream>
#include <vector>

#include <L3.h>
#include <code_generator.h>
#include <label_globalizer.h>

using namespace std;

namespace L3 {

const vector<string> argRegs = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

vector<string> generateAssign(string lhs, string rhs) { return {lhs + " <- " + rhs}; }

vector<string> generateCompare(string rst, string lhs, string op, string rhs) {
  return {rst + " <- " + lhs + " " + op + " " + rhs};
}

vector<string> generateLoad(string val, string addr) { return {val + " <- mem " + addr + " 0"}; }

vector<string> generateStore(string addr, string val) { return {"mem " + addr + " 0 <- " + val}; }

vector<string> generateArithmetic(const OperandNode *rst, const OperandNode *lhs,
                                  const ArithmeticNode *op, const OperandNode *rhs) {
  vector<string> code;
  auto result = rst->getOperand(), l = lhs->getOperand(), r = rhs->getOperand();

  if (op->getOp()->getID() == ArithOp::LS || op->getOp()->getID() == ArithOp::RS) {
    code.push_back("rcx <- " + r->toStr());
    r = new Variable("rcx");
  }

  // if rst equals to rhs, need to move rhs to another register
  if (result == r) {
    auto temp = new Variable("rcx");
    code.push_back(temp->toStr() + " <- " + r->toStr());
    r = temp;
  }

  // if rst equals to lhs, then no need to move lhs to rst
  if (result != l)
    code.push_back(result->toStr() + " <- " + l->toStr());

  code.push_back(result->toStr() + " " + op->toStr() + "= " + r->toStr());
  return code;
}

vector<string> generateBranch(string label) { return {"goto " + label}; }

vector<string> generateCondBranch(string cond, string label) {
  return {"cjump " + cond + " = 1 " + label};
}

vector<string> generateReturn() { return {"return"}; }

vector<string> generateReturnVal(string val) {
  auto code = generateAssign("rax", val);
  code.push_back("return");
  return code;
}

vector<string> generateCall(string callee, vector<string> args) {
  vector<string> code;
  auto labelName = LabelGlobalizer::generateNewName();
  code.push_back("mem rsp -8 <- " + labelName);
  for (int i = 0; i < min(6, (int)args.size()); i++)
    code.push_back(argRegs[i] + " <- " + args[i]);

  for (int i = 6; i < args.size(); i++)
    code.push_back("mem rsp -" + to_string(8 * (i - 4)) + " <- " + args[i]);

  code.push_back("call " + callee + " " + to_string(args.size()));
  code.push_back(labelName);
  return code;
}

vector<string> generateCallAssign(string rst, string callee, vector<string> args) {
  auto code = generateCall(callee, args);
  code.push_back(rst + " <- rax");
  return code;
}

vector<string> generateLabel(string label) { return {label + ":"}; }

void generate_code(const TilingResult &result, Program *P) {
  std::ofstream outputFile; // Use the fully qualified name for ofstream
  outputFile.open("prog.L2");

  outputFile << "(@main" << endl;
  for (auto F : P->getFunctions()) {
    int paramSize = F->getParams()->getParams().size();
    auto &paramList = F->getParams()->getParams();
    outputFile << "  (" << F->getName() << " " << F->getParams()->getParams().size() << endl;

    for (int i = 0; i < min(6, paramSize); i++)
      outputFile << "    " << paramList[i]->toStr() << " <- " << argRegs[i] << endl;
    for (int i = 6; i < paramSize; i++) {
      auto stackLoc = to_string(8 * (paramSize - i - 1));
      outputFile << "    " << paramList[i]->toStr() << " <- stack-arg " << stackLoc << endl;
    }

    auto &funcResult = result.at(F);
    auto &instructions = funcResult.assembleCode();
    for (string I : instructions)
      outputFile << "    " << I << endl;

    outputFile << "  )" << endl;
  }
  outputFile << ")" << endl;
}
} // namespace L3