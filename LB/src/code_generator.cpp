#include <fstream>

#include <LB.h>
#include <code_generator.h>
#include <helper.h>
#include <loop_analyzer.h>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

namespace LB {

class LACodeGenerator : public Visitor {
public:
  void visit(const DeclarationInst *inst) {
    for (auto var : inst->declaredVars->variables)
      LACodes.push_back(var->varType->getLAToken() + " " + var->getLAToken());
  }

  void visit(const AssignInst *inst) {
    LACodes.push_back(inst->getLhs()->getLAToken() + " <- " + inst->getRhs()->getLAToken());
  }

  void visit(const CmpInst *inst) {
    auto rst = inst->rst->getLAToken();
    auto lhs = inst->lhs->getLAToken();
    auto rhs = inst->rhs->getLAToken();
    auto op = inst->op->getLAToken();
    LACodes.push_back(rst + " <- " + lhs + " " + op + " " + rhs);
  }

  void visit(const OpInst *inst) {
    LACodes.push_back(inst->getRst()->getLAToken() + " <- " + inst->getLhs()->getLAToken() + " " +
                      inst->getOp()->getLAToken() + " " + inst->getRhs()->getLAToken());
  }

  void visit(const LoadInst *inst) {
    LACodes.push_back(inst->getTarget()->getLAToken() + " <- " + inst->getMemLoc()->getLAToken());
  }

  void visit(const StoreInst *inst) {
    LACodes.push_back(inst->getMemLoc()->getLAToken() + " <- " + inst->getSource()->getLAToken());
  }

  void visit(const ArrayLenInst *inst) {
    LACodes.push_back(inst->getResult()->getLAToken() + " <- length " + inst->getArray()->getLAToken() + " " +
                      inst->getDimIndex()->getLAToken());
  }

  void visit(const TupleLenInst *inst) {
    LACodes.push_back(inst->getResult()->getLAToken() + " <- length " + inst->getTuple()->getLAToken());
  }

  void visit(const NewArrayInst *inst) {
    string str = inst->getArray()->getLAToken() + " <- new Array(";
    for (auto size : inst->getSizes())
      str += size->getLAToken() + ", ";
    str = str.substr(0, str.size() - 2) + ")";
    LACodes.push_back(str);
  }

  void visit(const NewTupleInst *inst) {
    string str = inst->getTuple()->getLAToken() + " <- new Tuple(" + inst->getSize()->getLAToken() + ")";
    LACodes.push_back(str);
  }

  void visit(const IfInst *inst) {
    auto cond = F->getNewGlobVarName();
    LACodes.push_back("int64 " + cond);
    LACodes.push_back(cond + " <- " + inst->lhs->getLAToken() + " " + inst->op->getLAToken() + " " +
                      inst->rhs->getLAToken());
    LACodes.push_back("br " + cond + " " + inst->trueLabel->getLAToken() + " " + inst->falseLabel->getLAToken());
  }

  void visit(const WhileInst *inst) {
    auto cond = F->getNewGlobVarName();
    LACodes.push_back("int64 " + cond);
    LACodes.push_back(cond + " <- " + inst->lhs->getLAToken() + " " + inst->op->getLAToken() + " " +
                      inst->rhs->getLAToken());
    LACodes.push_back("br " + cond + " " + inst->bodyLabel->getLAToken() + " " + inst->exitLabel->getLAToken());
  }

  void visit(const ContinueInst *inst) {
    auto loop = loopInfo.loopMap.at(inst);
    auto labelName = loopInfo.loopLabels.at(loop)->getLAToken();
    LACodes.push_back("br " + labelName);
  }

  void visit(const BreakInst *inst) {
    auto loop = loopInfo.loopMap.at(inst);
    auto exitLabelName = loop->exitLabel->getLAToken();
    LACodes.push_back("br " + exitLabelName);
  }

  void visit(const RetInst *inst) { LACodes.push_back("return"); }

  void visit(const RetValueInst *inst) { LACodes.push_back("return " + inst->getValue()->getLAToken()); }

  void visit(const LabelInst *inst) { LACodes.push_back(inst->getLabel()->getLAToken()); }

  void visit(const GotoInst *inst) { LACodes.push_back("br " + inst->label->getLAToken()); }

  void visit(const CallInst *inst) {
    LACodes.push_back(inst->getCallee()->getLAToken() + "(" + inst->getArgs()->getLAToken() + ")");
  }

  void visit(const CallAssignInst *inst) {
    LACodes.push_back(inst->getRst()->getLAToken() + " <- " + inst->getCallee()->getLAToken() + "(" +
                      inst->getArgs()->getLAToken() + ")");
  }

  LACodeGenerator(Function *F, const LoopInfo &loopInfo) : F(F), loopInfo(loopInfo) {
    for (auto I : F->getInstructions())
      I->accept(*this);
  }

  const vector<string> &getLACodes() { return LACodes; }

private:
  vector<string> LACodes;
  Function *F;
  const LoopInfo &loopInfo;
};

void generate_code(Program *P, unordered_map<const Function *, const LoopInfo &> loopInfos) {
  std::ofstream outputFile;
  outputFile.open("prog.a");

  for (auto F : P->getFunctions()) {
    outputFile << F->getReturnType()->getLAToken() << " " << F->getName() << "(" << F->getParams()->getLAToken()
               << ") {" << endl;
    auto gen = LACodeGenerator(F, loopInfos.at(F));
    for (auto code : gen.getLACodes())
      outputFile << "  " << code << endl;
    outputFile << "}" << endl;
  }
}

} // namespace LB