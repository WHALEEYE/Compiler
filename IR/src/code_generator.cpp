#include <fstream>
#include <vector>
using namespace std;

#include <IR.h>
#include <code_generator.h>

namespace IR {

class GlobalVarNameGenerator {
public:
  GlobalVarNameGenerator(const Function *F) : counter(0) {
    string longestName = "var";
    for (auto &[varname, _] : F->getVariables()) {
      if (varname.size() > longestName.size())
        longestName = varname;
    }
    prefix = longestName + "_global";
  }

  string next() { return prefix + to_string(counter++); }

private:
  int64_t counter;
  string prefix;
};

class L3CodeGenerator : public Visitor {

  vector<string> encodeInsts(string result, const Value *encoded) {
    vector<string> insts;
    if (auto var = dynamic_cast<const Variable *>(encoded)) {
      insts.push_back(result + " <- " + var->toStr() + " << 1");
      insts.push_back(result + " <- " + result + " + 1");
    } else if (auto num = dynamic_cast<const Number *>(encoded)) {
      auto val = num->getValue();
      val = val * 2 + 1;
      insts.push_back(result + " <- " + to_string(val));
    }
    return insts;
  }

  vector<string> decodeInsts(string result, const Value *decoded) {
    vector<string> insts;
    if (auto var = dynamic_cast<const Variable *>(decoded)) {
      insts.push_back(result + " <- " + result + " >> 1");
    } else if (auto num = dynamic_cast<const Number *>(decoded)) {
      auto val = num->getValue();
      val = val - 1;
      val = val / 2;
      insts.push_back(result + " <- " + to_string(val));
    }
    return insts;
  }

  vector<string> getAddress(const MemoryLocation *memLoc, string addrVar) {
    vector<string> insts;
    auto &indices = memLoc->getIndices();
    auto baseType = memLoc->getBase()->getType();
    string offset = varNameGen->next();

    if (auto arrType = dynamic_cast<const ArrayType *>(baseType)) {
      vector<string> accums, decodedSizes;
      for (auto val : arrType->getSizes()) {
        string decodedSize = varNameGen->next();
        decodedSizes.push_back(decodedSize);
        vector<string> decodedInsts = decodeInsts(decodedSize, val);
        insts.insert(insts.end(), decodedInsts.begin(), decodedInsts.end());
      }

      string accum = varNameGen->next(), temp = varNameGen->next();
      insts.push_back(offset + " <- " + indices.back()->toStr());
      insts.push_back(accum + " <- " + decodedSizes.back());
      for (int i = indices.size() - 2; i >= 0; i--) {
        // add accum * index to offset
        insts.push_back(temp + " <- " + accum + " * " + indices[i]->toStr());
        insts.push_back(offset + " <- " + offset + " + " + temp);
        // update accum to accum * decodedSize
        insts.push_back(accum + " <- " + accum + " * " + decodedSizes[i]);
      }
      // add 1 + dim# to the offset
      insts.push_back(offset + " <- " + offset + " + " + to_string(arrType->getDim() + 1));
    } else if (auto tupleType = dynamic_cast<const TupleType *>(baseType)) {
      insts.push_back(offset + " <- " + indices.back()->toStr());
      // add 1 to the offset
      insts.push_back(offset + " <- " + offset + " + 1");
    } else {
      throw runtime_error("Invalid base type");
    }

    // multiply the offset by 8
    insts.push_back(offset + " <- " + offset + " << 3");
    // assign the address to base + offset
    insts.push_back(addrVar + " <- " + memLoc->getBase()->toStr() + " + " + offset);
    return insts;
  }

  void visit(const Variable *var) {}
  void visit(const Number *num) {}
  void visit(const MemoryLocation *mem) {}
  void visit(const Int64Type *type) {}
  void visit(const ArrayType *type) {}
  void visit(const TupleType *type) {}
  void visit(const CodeType *type) {}
  void visit(const VoidType *type) {}
  void visit(const Arguments *args) {}
  void visit(const Parameters *params) {}
  void visit(const CompareOp *op) {}
  void visit(const ArithOp *op) {}
  void visit(const RuntimeFunction *func) {}
  void visit(const FunctionName *name) {}
  void visit(const Label *label) {}
  void visit(const DeclarationInst *inst) {}
  void visit(const AssignInst *inst) {
    string instruction = inst->getLhs()->toStr() + " <- " + inst->getRhs()->toStr();
    instBuffer.push_back(instruction);
  }

  void visit(const ArithInst *inst) {
    string instruction = inst->getRst()->toStr() + " <- " + inst->getLhs()->toStr() + " " + inst->getOp()->toStr() +
                         " " + inst->getRhs()->toStr();
    instBuffer.push_back(instruction);
  }

  void visit(const CompareInst *inst) {
    string instruction = inst->getRst()->toStr() + " <- " + inst->getLhs()->toStr() + " " + inst->getOp()->toStr() +
                         " " + inst->getRhs()->toStr();
    instBuffer.push_back(instruction);
  }

  void visit(const LoadInst *inst) {
    auto addr = varNameGen->next();
    auto addrInsts = getAddress(inst->getMemLoc(), addr);
    instBuffer.insert(instBuffer.end(), addrInsts.begin(), addrInsts.end());
    instBuffer.push_back(inst->getTarget()->toStr() + " <- load " + addr);
  }

  void visit(const StoreInst *inst) {
    auto addr = varNameGen->next();
    auto addrInsts = getAddress(inst->getMemLoc(), addr);
    instBuffer.insert(instBuffer.end(), addrInsts.begin(), addrInsts.end());
    instBuffer.push_back("store " + addr + " " + inst->getSource()->toStr());
  }

  void visit(const ArrayLenInst *inst) {
    auto addr = varNameGen->next();
    auto dimIndex = inst->getDimIndex();
    auto offset = varNameGen->next();
    instBuffer.push_back(offset + " <- " + dimIndex->toStr() + " + 1");
    // multiply the offset by 8
    instBuffer.push_back(offset + " <- " + offset + " << 3");
    // assign the address to base + offset
    instBuffer.push_back(addr + " <- " + inst->getBase()->toStr() + " + " + offset);
    instBuffer.push_back(inst->getResult()->toStr() + " <- load " + addr);
  }

  void visit(const TupleLenInst *inst) {
    instBuffer.push_back(inst->getResult()->toStr() + " <- load " + inst->getBase()->toStr());
  }
  
  void visit(const NewArrayInst *inst) {}
  void visit(const NewTupleInst *inst) {}

  void visit(const RetInst *inst) { instBuffer.push_back("return"); }

  void visit(const RetValueInst *inst) { instBuffer.push_back("return " + inst->getValue()->toStr()); }

  void visit(const LabelInst *inst) { instBuffer.push_back(inst->getLabel()->toStr()); }

  void visit(const BranchInst *inst) { instBuffer.push_back("br " + inst->getLabel()->toStr()); }

  void visit(const CondBranchInst *inst) {
    if (inst->getFalseLabel() && inst->getTrueLabel()) {
      instBuffer.push_back("br " + inst->getCondition()->toStr() + " " + inst->getTrueLabel()->toStr());
      instBuffer.push_back("br " + inst->getFalseLabel()->toStr());
    } else if (inst->getFalseLabel()) {
      string name = varNameGen->next();
      instBuffer.push_back(name + " <- 1 - " + inst->getCondition()->toStr());
      instBuffer.push_back("br " + name + " " + inst->getFalseLabel()->toStr());
    } else if (inst->getTrueLabel()) {
      instBuffer.push_back("br " + inst->getCondition()->toStr() + " " + inst->getTrueLabel()->toStr());
    }
  }

  void visit(const CallInst *inst) {
    string instruction = "call " + inst->getCallee()->toStr() + "(";
    for (auto arg : inst->getArgs()->getArgs())
      instruction += arg->toStr() + ", ";
    if (inst->getArgs()->getArgs().size() > 0)
      instruction = instruction.substr(0, instruction.size() - 2);
    instruction += ")";
    instBuffer.push_back(instruction);
  }

  void visit(const CallAssignInst *inst) {
    string instruction = inst->getRst()->toStr() + " <- call " + inst->getCallee()->toStr() + "(";
    for (auto arg : inst->getArgs()->getArgs())
      instruction += arg->toStr() + ", ";
    if (inst->getArgs()->getArgs().size() > 0)
      instruction = instruction.substr(0, instruction.size() - 2);
    instruction += ")";
    instBuffer.push_back(instruction);
  }

  void doVisit(const Instruction *inst) {
    instBuffer.clear();
    inst->accept(*this);
    instructions.insert(instructions.end(), instBuffer.begin(), instBuffer.end());
  }

  L3CodeGenerator(Function *F) {
    instructions = {};
    instBuffer = {};
    varNameGen = new GlobalVarNameGenerator(F);
  }

private:
  vector<string> instructions;
  vector<string> instBuffer;
  GlobalVarNameGenerator *varNameGen;
};

void generate_code(Program *P) {
  std::ofstream outputFile;
  outputFile.open("prog.L3");

  for (auto F : P->getFunctions()) {
    auto codeGen = GlobalVarNameGenerator(F);
    auto paramSize = (int)F->getParams()->getParams().size();
    auto &paramList = F->getParams()->getParams();
    outputFile << "  (" << F->getName() << " " << F->getParams()->getParams().size() << endl;

    for (int i {} i < min(6, paramSize); i++)
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
}
} // namespace IR