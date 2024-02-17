#include <algorithm>
#include <fstream>
#include <utility>
#include <vector>

#include <code_generator.h>
#include <label_globalizer.h>

using namespace std;

namespace L3 {

const vector<string> argRegs = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

vector<string> generateAssign(const string &lhs, const string &rhs) { return {lhs + " <- " + rhs}; }

vector<string> generateCompare(const string &rst, const string &lhs, const string &op, const string &rhs) {
  return {rst + " <- " + lhs + " " + op + " " + rhs};
}

vector<string> generateLoad(const string &val, const string &addr) { return {val + " <- mem " + addr + " 0"}; }

vector<string> generateStore(const string &addr, const string &val) { return {"mem " + addr + " 0 <- " + val}; }

vector<string> generateArithmetic(const OperandNode *rst, const OperandNode *lhs, const ArithmeticNode *op,
                                  const OperandNode *rhs) {
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

vector<string> generateBranch(const string &label) { return {"goto " + label}; }

vector<string> generateCondBranch(const string &cond, const string &label) {
  return {"cjump " + cond + " = 1 " + label};
}

vector<string> generateReturn() { return {"return"}; }

vector<string> generateReturnVal(const string &val) {
  auto code = generateAssign("rax", val);
  code.emplace_back("return");
  return code;
}

vector<string> generateCall(const string &callee, vector<string> args) {
  vector<string> code;
  auto labelName = LabelGlobalizer::generateNewName();
  code.push_back("mem rsp -8 <- " + labelName);
  for (int i = 0; i < min(6, (int)args.size()); i++)
    code.push_back(argRegs[i] + " <- " + args[i]);
programTilingResult
  for (int i = 6; i < args.size(); i++)
    code.push_back("mem rsp -" + to_string(8 * (i - 4)) + " <- " + args[i]);

  code.push_back("call " + callee + " " + to_string(args.size()));
  code.push_back(labelName);
  return code;
}

vector<string> generateCallAssign(const string &rst, const string &callee, vector<string> args) {
  auto code = generateCall(callee, std::move(args));
  code.push_back(rst + " <- rax");
  return code;
}

vector<string> generateLabel(const string &label) { return {label + ":"}; }

void generate_code(const unordered_map<const Function *, const TilingResult &> &result, Program *P) {
  std::ofstream outputFile; // Use the fully qualified name for ofstream
  outputFile.open("prog.L2");

  outputFile << "(@main" << endl;
  for (auto F : P->getFunctions()) {
    auto paramSize = (int)F->getParams()->getParams().size();
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
    for (const string &I : instructions)
      outputFile << "    " << I << endl;

    outputFile << "  )" << endl;
  }
  outputFile << ")" << endl;
}
} // namespace L3