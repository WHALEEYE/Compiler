#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include <IR.h>
#include <code_generator.h>
#include <helper.h>

namespace IR {

class GlobalVarNameGenerator {
public:
  GlobalVarNameGenerator(const Function *F) : counter(0) {
    string longestName = "%var";
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
public:
  vector<string> generateEncodeInstructions(string result, const Value *encoded) {
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

  vector<string> generateDecodeInstructions(string result, const Value *decoded) {
    vector<string> insts;
    if (auto var = dynamic_cast<const Variable *>(decoded)) {
      insts.push_back(result + " <- " + var->toStr() + " >> 1");
    } else if (auto num = dynamic_cast<const Number *>(decoded)) {
      auto val = num->getValue();
      val = val - 1;
      val = val / 2;
      insts.push_back(result + " <- " + to_string(val));
    }
    return insts;
  }

  vector<string> generateEncodeInstructions(string encodedVar) {
    vector<string> insts;
    insts.push_back(encodedVar + " <- " + encodedVar + " << 1");
    insts.push_back(encodedVar + " <- " + encodedVar + " + 1");
    return insts;
  }

  vector<string> generateDecodeInstructions(string decodedVar) {
    vector<string> insts;
    insts.push_back(decodedVar + " <- " + decodedVar + " >> 1");
    return insts;
  }

  vector<string> getL3Address(const MemoryLocation *memLoc, string addrVar) {
    vector<string> insts;
    auto &indices = memLoc->getIndices();
    auto baseType = memLoc->getBase()->getType();
    string offset = varNameGen->next();

    if (auto arrType = dynamic_cast<const ArrayType *>(baseType)) {
      auto sizePtr = varNameGen->next();
      insts.push_back(sizePtr + " <- " + memLoc->getBase()->toStr());
      vector<string> accums, decodedSizes;
      for (int i = 0; i < arrType->getDim(); i++) {
        insts.push_back(sizePtr + " <- " + sizePtr + " + 8");
        string decodedSize = varNameGen->next();
        decodedSizes.push_back(decodedSize);
        insts.push_back(decodedSize + " <- load " + sizePtr);
        vector<string> decodedInsts = generateDecodeInstructions(decodedSize);
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
    auto addrInsts = getL3Address(inst->getMemLoc(), addr);
    instBuffer.insert(instBuffer.end(), addrInsts.begin(), addrInsts.end());
    instBuffer.push_back(inst->getTarget()->toStr() + " <- load " + addr);
  }

  void visit(const StoreInst *inst) {
    auto addr = varNameGen->next();
    auto addrInsts = getL3Address(inst->getMemLoc(), addr);
    instBuffer.insert(instBuffer.end(), addrInsts.begin(), addrInsts.end());
    instBuffer.push_back("store " + addr + " <- " + inst->getSource()->toStr());
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
    auto result = inst->getResult()->toStr();
    instBuffer.push_back(result + " <- load " + inst->getBase()->toStr());
    // encode the result
    auto encodeInsts = generateEncodeInstructions(result);
    instBuffer.insert(instBuffer.end(), encodeInsts.begin(), encodeInsts.end());
  }

  void visit(const NewArrayInst *inst) {

    if (!dynamic_cast<const ArrayType *>(inst->getArray()->getType()))
      throw runtime_error("Invalid type for NewArrayInst");

    auto array = inst->getArray();
    auto arrayType = dynamic_cast<const ArrayType *>(array->getType());
    auto sizes = inst->getSizes();

    // calculate the size of the array: dim# + size1 * size2 * ... * sizeN
    auto size = varNameGen->next();
    for (int i = 0; i < sizes.size(); i++) {
      // decode the size first
      auto decodedSize = varNameGen->next();
      auto decodeInsts = generateDecodeInstructions(decodedSize, sizes[i]);
      instBuffer.insert(instBuffer.end(), decodeInsts.begin(), decodeInsts.end());

      if (i == 0)
        instBuffer.push_back(size + " <- " + decodedSize);
      else
        instBuffer.push_back(size + " <- " + size + " * " + decodedSize);
    }
    instBuffer.push_back(size + " <- " + size + " + " + to_string(arrayType->getDim()));
    auto encodeInsts = generateEncodeInstructions(size);
    instBuffer.insert(instBuffer.end(), encodeInsts.begin(), encodeInsts.end());
    // allocate the memory
    instBuffer.push_back(array->toStr() + " <- call allocate(" + size + ", 1)");
    // store the size of the array
    auto sizePtr = varNameGen->next();
    instBuffer.push_back(sizePtr + " <- " + array->toStr());
    for (auto size : sizes) {
      instBuffer.push_back(sizePtr + " <- " + sizePtr + " + 8");
      instBuffer.push_back("store " + sizePtr + " <- " + size->toStr());
    }
  }

  void visit(const NewTupleInst *inst) {
    auto tuple = inst->getTuple();
    auto size = inst->getSize();
    // allocate the memory
    instBuffer.push_back(tuple->toStr() + " <- call allocate(" + size->toStr() + ", 1)");
  }

  void visit(const RetInst *inst) { instBuffer.push_back("return"); }

  void visit(const RetValueInst *inst) { instBuffer.push_back("return " + inst->getValue()->toStr()); }

  void visit(const LabelInst *inst) {
    debug("parsing label: " + inst->toStr());
    instBuffer.push_back(inst->getLabel()->toStr());
  }

  void visit(const BranchInst *inst) { instBuffer.push_back("br " + inst->getLabel()->toStr()); }

  void visit(const CondBranchInst *inst) {
    instBuffer.push_back("br " + inst->getCondition()->toStr() + " " + inst->getTrueLabel()->toStr());
    if (inst->getFalseLabel() != nullptr)
      instBuffer.push_back("br " + inst->getFalseLabel()->toStr());
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

  const vector<string> &getInstructions() const { return instructions; }

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
    L3CodeGenerator codeGen(F);
    auto paramSize = (int)F->getParams()->getParams().size();
    auto &paramList = F->getParams()->getParams();
    string paramStr = "";
    for (int i = 0; i < paramSize; i++) {
      paramStr += paramList[i]->toStr();
      if (i != paramSize - 1)
        paramStr += ", ";
    }
    outputFile << "define " << F->getName() << "(" << paramStr << ") {" << endl;

    for (auto BB : F->getBasicBlocks())
      for (auto I : BB->getInstructions())
        codeGen.doVisit(I);

    for (auto inst : codeGen.getInstructions())
      outputFile << "  " << inst << endl;

    outputFile << "}\n" << endl;
  }
}
} // namespace IR