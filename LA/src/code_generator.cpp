#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include <LA.h>
#include <code_generator.h>
#include <helper.h>

namespace LA {

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

class IRGenerator : public Visitor {
public:
  void visit(const DeclarationInst *inst) {
    auto var = inst->getVar();
    currentInsts->push_back(var->getVarType()->toStr() + " " + var->getPrefixedName());
    auto type = var->getVarType()->getType();
    int64_t initValue;
    if (type == VarType::Type::INT64)
      initValue = 1;
    else if (type == VarType::Type::TUPLE || type == VarType::Type::ARRAY || type == VarType::Type::CODE)
      initValue = 0;
    else
      throw runtime_error("void type variable cannot be initialized");
    auto initInst = var->getPrefixedName() + " <- " + to_string(initValue);
    currentInsts->push_back(initInst);
  }

  void visit(const AssignInst *inst) {
    // if right side is a constant, encode it
    currentInsts->push_back(inst->getLhs()->getPrefixedName() + " <- " + getEncodedUse(inst->getRhs()));
  }

  void visit(const OpInst *inst) {
    // decode all the operands
    auto rst = inst->getRst()->getPrefixedName();
    auto lhs = getDecodedUse(inst->getLhs()), rhs = getDecodedUse(inst->getRhs());
    string instruction = rst + " <- " + lhs + " " + inst->getOp()->toStr() + " " + rhs;
    currentInsts->push_back(instruction);
    currentInsts->push_back(rst + " <- " + rst + " << 1");
    currentInsts->push_back(rst + " <- " + rst + " + 1");
  }

  void visit(const LoadInst *inst) {
    // generate error handler codes
    currentInsts->push_back(errorLine + " <- " + to_string(inst->lineno));

    auto target = inst->getTarget()->getPrefixedName();
    auto memStr = getIRMemLocWithCheck(inst->getMemLoc(), inst->lineno);
    currentInsts->push_back(target + " <- " + memStr);
  }

  void visit(const StoreInst *inst) {
    auto memStr = getIRMemLocWithCheck(inst->getMemLoc(), inst->lineno);
    currentInsts->push_back(memStr + " <- " + getEncodedUse(inst->getSource()));
  }

  void visit(const ArrayLenInst *inst) {
    auto rst = inst->getResult()->getPrefixedName();
    auto arr = inst->getArray()->getPrefixedName();
    auto dimInd = getDecodedUse(inst->getDimIndex());
    currentInsts->push_back(rst + " <- length " + arr + " " + dimInd);
  }

  void visit(const TupleLenInst *inst) {
    auto rst = inst->getResult()->getPrefixedName();
    auto tuple = inst->getTuple()->getPrefixedName();
    currentInsts->push_back(rst + " <- length " + tuple);
  }

  void visit(const NewArrayInst *inst) {
    auto str = inst->getArray()->getPrefixedName() + " <- new Array(";
    for (auto size : inst->getSizes())
      str += getEncodedUse(size) + ", ";
    str = str.substr(0, str.size() - 2);
    str += ")";
    currentInsts->push_back(str);
  }

  void visit(const NewTupleInst *inst) {
    auto tuple = inst->getTuple()->getPrefixedName();
    auto size = getEncodedUse(inst->getSize());
    currentInsts->push_back(tuple + " <- new Tuple(" + size + ")");
  }

  void visit(const RetInst *inst) { currentInsts->push_back("return"); }

  void visit(const RetValueInst *inst) { currentInsts->push_back("return " + getEncodedUse(inst->getValue())); }

  void visit(const LabelInst *inst) { currentInsts->push_back(inst->getLabel()->toStr()); }

  void visit(const BranchInst *inst) { currentInsts->push_back("br " + inst->getLabel()->toStr()); }

  void visit(const CondBranchInst *inst) {
    auto cond = getDecodedUse(inst->getCondition());
    currentInsts->push_back("br " + cond + " " + inst->getTrueLabel()->toStr() + " " + inst->getFalseLabel()->toStr());
  }

  void visit(const CallInst *inst) {
    auto callee = inst->getCallee()->getPrefixedName();
    auto args = getIRArguments(inst->getArgs());
    currentInsts->push_back("call " + callee + "(" + args + ")");
  }

  void visit(const CallAssignInst *inst) {
    auto rst = inst->getRst()->getPrefixedName();
    auto callee = inst->getCallee()->getPrefixedName();
    auto args = getIRArguments(inst->getArgs());
    currentInsts->push_back(rst + " <- call " + callee + "(" + args + ")");
  }

  vector<string> getInstructions() const {
    vector<string> insts = entryInsts;
    insts.insert(insts.end(), bodyInsts.begin(), bodyInsts.end());
    return insts;
  }

  IRGenerator(Function *F) {
    this->F = F;
    bodyInsts = {};
    entryInsts = {};
    tsErrorHandler1 = F->generateNewLabelName();
    tsErrorHandler3 = F->generateNewLabelName();
    tsErrorHandler4 = F->generateNewLabelName();
    tpErrorHandler3 = F->generateNewLabelName();
    errorLine = "%" + F->generateNewVariableName();
    errorDim = "%" + F->generateNewVariableName();
    errorLen = "%" + F->generateNewVariableName();
    errorIndex = "%" + F->generateNewVariableName();
    errorCheck = "%" + F->generateNewVariableName();
    auto entryBB = F->getBasicBlocks().front();

    // generate the rest of the blocks
    currentInsts = &bodyInsts;
    for (auto BB : F->getBasicBlocks()) {
      if (BB == entryBB)
        continue;
      currentInsts->push_back(BB->getLabel()->toStr());
      for (auto I : BB->getInstructions())
        I->accept(*this);
      BB->getTerminator()->accept(*this);
      currentInsts->push_back("");
    }

    // handle the entry block, which has already contained some declarations
    // insert the label at the beginning
    entryInsts.insert(entryInsts.begin(), entryBB->getLabel()->toStr());
    // insert the declarations of error handler variables
    entryInsts.push_back("int64 " + errorLine);
    entryInsts.push_back("int64 " + errorDim);
    entryInsts.push_back("int64 " + errorLen);
    entryInsts.push_back("int64 " + errorIndex);
    entryInsts.push_back("int64 " + errorCheck);
    currentInsts = &entryInsts;
    for (auto I : entryBB->getInstructions())
      I->accept(*this);
    entryBB->getTerminator()->accept(*this);
    entryInsts.push_back("");
    // insert the error handlers
    // error handler with 1 argument
    entryInsts.push_back(tsErrorHandler1);
    entryInsts.push_back("call tensor-error(" + errorLine + ")");
    entryInsts.push_back("return\n");
    // error handler with 3 arguments
    entryInsts.push_back(tsErrorHandler3);
    entryInsts.push_back("call tensor-error(" + errorLine + ", " + errorLen + ", " + errorIndex + ")");
    entryInsts.push_back("return\n");
    // error handler with 4 arguments
    entryInsts.push_back(tsErrorHandler4);
    entryInsts.push_back("call tensor-error(" + errorLine + ", " + errorDim + ", " + errorLen + ", " + errorIndex +
                         ")");
    entryInsts.push_back("return\n");
    // error handler for tuple
    entryInsts.push_back(tpErrorHandler3);
    entryInsts.push_back("call tuple-error(" + errorLine + ", " + errorLen + ", " + errorIndex + ")");
    entryInsts.push_back("return\n");
  }

private:
  Function *F;
  vector<string> entryInsts;
  vector<string> bodyInsts;
  vector<string> *currentInsts;

  // label for the error handler
  string tsErrorHandler1, tsErrorHandler3, tsErrorHandler4, tpErrorHandler3;
  string errorLine, errorDim, errorLen, errorIndex, errorCheck;

  /*
   * Get the decoded form of a value. Only numbers and variables (not code or void type) can be decoded.
   * If the value is a number, return the number itself because it is originally decoded.
   * If the value is a function pointer, throw an error because it cannot be decoded.
   * If the value is a variable, decode it and return the decoded variable name. It will also insert the declaration
   * instruction into the entry block.
   */
  string getDecodedUse(const Value *val) {
    // if a number is sent to this function, no need to decode it
    if (dynamic_cast<const Number *>(val))
      return val->toStr();
    else if (dynamic_cast<const UserFunction *>(val) || dynamic_cast<const RuntimeFunction *>(val))
      throw runtime_error("function pointers cannot be decoded");

    auto var = dynamic_cast<const Variable *>(val);
    auto type = var->getVarType()->getType();

    if (type == VarType::Type::CODE || type == VarType::Type::VOID)
      throw runtime_error("code / void type variable cannot be decoded");

    auto decodedVar = "%" + F->generateNewVariableName();
    // add the declaration instruction to the entry block
    entryInsts.push_back(var->getVarType()->toStr() + " " + decodedVar);
    currentInsts->push_back(decodedVar + " <- " + var->getPrefixedName() + " >> 1");
    return decodedVar;
  }

  /*
   * Get the encoded form of a value.
   * If the value is a number, return the encoded number.
   * Else, return the prefixed version of the name.
   */
  string getEncodedUse(const Value *val) {
    if (dynamic_cast<const Number *>(val)) {
      auto num = dynamic_cast<const Number *>(val);
      auto val = num->getValue();
      return to_string(val * 2 + 1);
    } else {
      auto name = dynamic_cast<const Name *>(val);
      return name->getPrefixedName();
    }
  }

  string getIRMemLocWithCheck(const MemoryLocation *mem, int64_t lineno) {
    auto pointer = mem->getBase();
    string nextLabel;
    currentInsts->push_back(errorLine + " <- " + to_string(lineno * 2 + 1));
    // check if the pointer is null
    currentInsts->push_back(errorCheck + " <- " + pointer->getPrefixedName() + " = 0");
    auto normalLabel = F->generateNewLabelName();
    currentInsts->push_back("br " + errorCheck + " " + tsErrorHandler1 + " " + normalLabel + "\n");
    currentInsts->push_back(normalLabel);

    // store all the decoded strings
    vector<string> decodedIndices;
    auto dim = mem->getIndices().size();

    if (dim > 1) {
      auto arrType = dynamic_cast<const ArrayType *>(pointer->getVarType());
      for (int i = 0; i < dim; i++) {
        // store the current dimension into errorDim
        currentInsts->push_back(errorDim + " <- " + to_string(i * 2 + 1));
        // store the length of current dimension into temp var
        currentInsts->push_back(errorLen + " <- length " + pointer->getPrefixedName() + " " + to_string(i));
        // store the encoded index into errorIndex
        currentInsts->push_back(errorIndex + " <- " + getEncodedUse(mem->getIndices()[i]));
        // check if the index is smaller than 0
        currentInsts->push_back(errorCheck + " <- " + errorIndex + " < 1");
        nextLabel = F->generateNewLabelName();
        currentInsts->push_back("br " + errorCheck + " " + tsErrorHandler4 + " " + nextLabel + "\n");
        currentInsts->push_back(nextLabel);
        // check if the index is larger than the length
        currentInsts->push_back(errorCheck + " <- " + errorLen + " <= " + errorIndex);
        nextLabel = F->generateNewLabelName();
        currentInsts->push_back("br " + errorCheck + " " + tsErrorHandler4 + " " + nextLabel + "\n");
        currentInsts->push_back(nextLabel);
      }
    } else {
      // only one dimension
      bool isTuple = pointer->getVarType()->getType() == VarType::Type::TUPLE;
      auto errorHandler = isTuple ? tpErrorHandler3 : tsErrorHandler3;
      currentInsts->push_back(errorLen + " <- length " + pointer->getPrefixedName() + (isTuple ? "" : " 0"));
      // store the encoded index into errorIndex
      currentInsts->push_back(errorIndex + " <- " + getEncodedUse(mem->getIndices()[0]));
      // check if the index is smaller than 0
      currentInsts->push_back(errorCheck + " <- " + errorIndex + " < 1");
      nextLabel = F->generateNewLabelName();
      currentInsts->push_back("br " + errorCheck + " " + errorHandler + " " + nextLabel + "\n");
      currentInsts->push_back(nextLabel);
      // check if the index is larger than the length
      currentInsts->push_back(errorCheck + " <- " + errorLen + " <= " + errorIndex);
      nextLabel = F->generateNewLabelName();
      currentInsts->push_back("br " + errorCheck + " " + errorHandler + " " + nextLabel + "\n");
      currentInsts->push_back(nextLabel);
    }

    // all the indices should be in decoded version
    auto str = mem->getBase()->getPrefixedName();
    for (auto index : mem->getIndices())
      str += "[" + getDecodedUse(index) + "]";
    return str;
  }

  string getIRArguments(const Arguments *args) {
    // all arguments should be in encoded version
    string str = "";
    for (auto arg : args->getArgs())
      str += getEncodedUse(arg) + ", ";
    if (args->getArgs().size() > 0)
      str = str.substr(0, str.size() - 2);
    return str;
  }

  void generateErrorChecker(const MemoryLocation *memLoc, int64_t lineno) {
    auto pointer = memLoc->getBase();
    currentInsts->push_back(errorLine + " <- " + to_string(lineno * 2 + 1));
    // check if the pointer is null
    currentInsts->push_back(errorCheck + " <- " + pointer->getPrefixedName() + " = 0");
    auto normalLabel = F->generateNewLabelName();
    currentInsts->push_back("br " + errorCheck + " " + tsErrorHandler1 + " " + normalLabel + "\n");
  }
};

void generate_code(Program *P) {
  std::ofstream outputFile;
  outputFile.open("prog.IR");

  for (auto F : P->getFunctions()) {
    IRGenerator codeGen(F);

    string params = "";
    for (auto param : F->getParams()->getParams())
      params += param->getVarType()->toStr() + " %" + param->toStr() + ", ";
    if (F->getParams()->getParams().size() > 0)
      params = params.substr(0, params.size() - 2);
    outputFile << "define " << F->getReturnType()->toStr() << " @" << F->getName() << "(" << params << ") {" << endl;

    for (auto inst : codeGen.getInstructions())
      outputFile << "  " << inst << endl;

    outputFile << "}\n" << endl;
  }
}

} // namespace LA