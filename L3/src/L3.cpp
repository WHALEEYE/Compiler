#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std;

#include <L3.h>

namespace L3 {

std::string Item::toStr() const { throw runtime_error("Item::toStr() not implemented"); }
void Item::accept(Visitor &visitor) const { throw runtime_error("Item::accept() not implemented"); }

Variable::Variable(string name) : name(name) {}
string Variable::toStr() const { return name; }
void Variable::accept(Visitor &visitor) const { visitor.visit(this); }

Number::Number(int64_t val) : val{val} {}
int64_t Number::getVal() const { return val; }
string Number::toStr() const { return to_string(val); }
void Number::accept(Visitor &visitor) const { visitor.visit(this); }

const LeftParen *LeftParen::instance = new LeftParen();
const LeftParen *LeftParen::getInstance() { return instance; }

const RightParen *RightParen::instance = new RightParen();
const RightParen *RightParen::getInstance() { return instance; }

void Arguments::addArgToHead(const Value *arg) { args.insert(args.begin(), arg); }
const vector<const Value *> &Arguments::getArgs() const { return args; }
string Arguments::toStr() const {
  string str;

  for (auto arg : args)
    str += arg->toStr() + ", ";

  return str == "" ? str : str.substr(0, str.size() - 2);
}
void Arguments::accept(Visitor &visitor) const { visitor.visit(this); }

void Parameters::addParamToHead(const Variable *param) { params.insert(params.begin(), param); }
const vector<const Variable *> &Parameters::getParams() const { return params; }
string Parameters::toStr() const {
  string str;

  for (auto param : params)
    str += param->toStr() + ", ";

  return str == "" ? str : str.substr(0, str.size() - 2);
}
void Parameters::accept(Visitor &visitor) const { visitor.visit(this); }

CompareOp::CompareOp(string name) : name{name} {}
CompareOp *CompareOp::getCompareOp(ID id) { return enumMap.at(id); }
string CompareOp::getName() const { return name; }
string CompareOp::toStr() const { return name; }
void CompareOp::accept(Visitor &visitor) const { visitor.visit(this); }
const unordered_map<CompareOp::ID, CompareOp *> CompareOp::enumMap = {
    {ID::LESS_THAN, new CompareOp("<")},
    {ID::LESS_EQUAL, new CompareOp("<=")},
    {ID::EQUAL, new CompareOp("=")},
    {ID::GREATER_EQUAL, new CompareOp(">=")},
    {ID::GREATER_THAN, new CompareOp(">")}};

ArithOp::ArithOp(string name) : name{name} {}
ArithOp *ArithOp::getArithOp(ID id) { return enumMap.at(id); }
string ArithOp::getName() const { return name; }
string ArithOp::toStr() const { return name; }
void ArithOp::accept(Visitor &visitor) const { visitor.visit(this); }
const unordered_map<ArithOp::ID, ArithOp *> ArithOp::enumMap = {
    {ID::ADD, new ArithOp("+")}, {ID::SUB, new ArithOp("-")}, {ID::MUL, new ArithOp("*")},
    {ID::AND, new ArithOp("&")}, {ID::LS, new ArithOp("<<")}, {ID::RS, new ArithOp(">>")}};

RuntimeFunction::RuntimeFunction(string name) : name{name} {}
RuntimeFunction *RuntimeFunction::getRuntimeFunction(ID id) { return enumMap.at(id); }
string RuntimeFunction::getName() const { return name; }
string RuntimeFunction::toStr() const { return name; }
void RuntimeFunction::accept(Visitor &visitor) const { visitor.visit(this); }
const unordered_map<RuntimeFunction::ID, RuntimeFunction *> RuntimeFunction::enumMap = {
    {ID::PRINT, new RuntimeFunction("print")},
    {ID::ALLOCATE, new RuntimeFunction("allocate")},
    {ID::INPUT, new RuntimeFunction("input")},
    {ID::TUPLE_ERROR, new RuntimeFunction("tuple-error")},
    {ID::TENSOR_ERROR, new RuntimeFunction("tensor-error")}};

FunctionName::FunctionName(string name) : name{name} {}
string FunctionName::getName() const { return name; }
string FunctionName::toStr() const { return name; }
void FunctionName::accept(Visitor &visitor) const { visitor.visit(this); }

Label::Label(string name) : name{name} {}
string Label::getName() const { return name; }
string Label::toStr() const { return name; }
void Label::accept(Visitor &visitor) const { visitor.visit(this); }

/*
 *  Instructions.
 */
AssignInst::AssignInst(const Variable *lval, const Item *rval) : lval{lval}, rval{rval} {}
const Variable *AssignInst::getLval() const { return lval; }
const Item *AssignInst::getRval() const { return rval; }
string AssignInst::toStr() const { return lval->toStr() + " <- " + rval->toStr(); }
void AssignInst::accept(Visitor &visitor) const { visitor.visit(this); }

ArithInst::ArithInst(const Variable *lval, const Value *arithLval, const ArithOp *op,
                     const Value *arithRval)
    : lval(lval), arithLval(arithLval), op(op), arithRval(arithRval) {}
const Variable *ArithInst::getLval() const { return lval; }
const Value *ArithInst::getArithLval() const { return arithLval; }
const ArithOp *ArithInst::getOp() const { return op; }
const Value *ArithInst::getArithRval() const { return arithRval; }
string ArithInst::toStr() const {
  return lval->toStr() + " <- " + arithLval->toStr() + " " + op->toStr() + " " + arithRval->toStr();
}
void ArithInst::accept(Visitor &visitor) const { visitor.visit(this); }

CompareInst::CompareInst(const Variable *lval, const Value *cmpLval, const CompareOp *op,
                         const Value *cmpRval)
    : lval{lval}, cmpLval{cmpLval}, op{op}, cmpRval{cmpRval} {}
const Variable *CompareInst::getLval() const { return lval; }
const CompareOp *CompareInst::getOp() const { return op; }
const Value *CompareInst::getCmpLval() const { return cmpLval; }
const Value *CompareInst::getCmpRval() const { return cmpRval; }
string CompareInst::toStr() const {
  return lval->toStr() + " <- " + cmpLval->toStr() + " " + op->toStr() + " " + cmpRval->toStr();
}
void CompareInst::accept(Visitor &visitor) const { visitor.visit(this); }

LoadInst::LoadInst(const Variable *lval, const Variable *addr) : lval{lval}, addr{addr} {}
const Variable *LoadInst::getLval() const { return lval; }
const Variable *LoadInst::getAddr() const { return addr; }
string LoadInst::toStr() const { return lval->toStr() + " <- load " + addr->toStr(); }
void LoadInst::accept(Visitor &visitor) const { visitor.visit(this); }

StoreInst::StoreInst(const Variable *addr, const Value *rval) : addr{addr}, rval{rval} {}
const Variable *StoreInst::getAddr() const { return addr; }
const Value *StoreInst::getRval() const { return rval; }
string StoreInst::toStr() const { return "store " + addr->toStr() + " <- " + rval->toStr(); }
void StoreInst::accept(Visitor &visitor) const { visitor.visit(this); }

string RetInst::toStr() const { return "return"; }
void RetInst::accept(Visitor &visitor) const { visitor.visit(this); }

RetValueInst::RetValueInst(const Value *val) : val{val} {}
const Value *RetValueInst::getVal() const { return val; }
string RetValueInst::toStr() const { return "return " + val->toStr(); }
void RetValueInst::accept(Visitor &visitor) const { visitor.visit(this); }

LabelInst::LabelInst(const Label *label) : label{label} {}
const Label *LabelInst::getLabel() const { return label; }
string LabelInst::toStr() const { return label->toStr(); }
void LabelInst::accept(Visitor &visitor) const { visitor.visit(this); }

BranchInst::BranchInst(const Label *label) : label{label} {}
const Label *BranchInst::getLabel() const { return label; }
string BranchInst::toStr() const { return "br " + label->toStr(); }
void BranchInst::accept(Visitor &visitor) const { visitor.visit(this); }

CondBranchInst::CondBranchInst(const Value *condition, const Label *label)
    : condition{condition}, label{label} {}
const Value *CondBranchInst::getCondition() const { return condition; }
const Label *CondBranchInst::getLabel() const { return label; }
string CondBranchInst::toStr() const { return "br " + condition->toStr() + " " + label->toStr(); }
void CondBranchInst::accept(Visitor &visitor) const { visitor.visit(this); }

CallInst::CallInst(const Item *callee, const Arguments *args) : callee{callee}, args{args} {}
const Item *CallInst::getCallee() const { return callee; }
const Arguments *CallInst::getArgs() const { return args; }
string CallInst::toStr() const { return "call " + callee->toStr() + "(" + args->toStr() + ")"; }
void CallInst::accept(Visitor &visitor) const { visitor.visit(this); }

CallAssignInst::CallAssignInst(const Variable *lval, const Item *callee, const Arguments *args)
    : lval{lval}, callee{callee}, args{args} {}
const Variable *CallAssignInst::getLval() const { return lval; }
const Item *CallAssignInst::getCallee() const { return callee; }
const Arguments *CallAssignInst::getArgs() const { return args; }
string CallAssignInst::toStr() const {
  return lval->toStr() + " <- call " + callee->toStr() + "(" + args->toStr() + ")";
}
void CallAssignInst::accept(Visitor &visitor) const { visitor.visit(this); }

const vector<const Instruction *> &BasicBlock::getInstructions() const { return instructions; }
void BasicBlock::addInstruction(const Instruction *inst) { instructions.push_back(inst); }
const unordered_set<BasicBlock *> &BasicBlock::getSuccessors() const { return successors; }
void BasicBlock::addSuccessor(BasicBlock *BB) { successors.insert(BB); }
void BasicBlock::removeSuccessor(BasicBlock *BB) { successors.erase(BB); }
const unordered_set<BasicBlock *> &BasicBlock::getPredecessors() const { return predecessors; }
void BasicBlock::addPredecessor(BasicBlock *BB) { predecessors.insert(BB); }
void BasicBlock::removePredecessor(BasicBlock *BB) { predecessors.erase(BB); }
const Instruction *BasicBlock::getFirstInstruction() const { return instructions.front(); }
const Instruction *BasicBlock::getTerminator() const { return instructions.back(); }
string BasicBlock::toStr() const {
  string str;
  for (auto inst : instructions)
    str += "  " + inst->toStr() + "\n";
  return str;
}

Function::Function(string name) : name{name} {
  // start with an empty basic block
  basicBlocks.push_back(new BasicBlock());
}
string Function::getName() const { return name; }
const Parameters *Function::getParams() const { return params; }
void Function::setParams(const Parameters *params) { this->params = params; }
const vector<BasicBlock *> &Function::getBasicBlocks() const { return basicBlocks; }
void Function::addBasicBlock(BasicBlock *BB) { basicBlocks.push_back(BB); }
BasicBlock *Function::getCurrBasicBlock() const { return basicBlocks.back(); }
void Function::popCurrBasicBlock() { basicBlocks.pop_back(); }
const Variable *Function::getVariable(string name) {
  if (variables.find(name) == variables.end())
    variables[name] = new Variable(name);
  return variables[name];
}
bool Function::hasVariable(string name) const { return variables.find(name) != variables.end(); }
const unordered_map<string, const Variable *> &Function::getVariables() const { return variables; }
string Function::toStr() const {
  string str;
  str += "define " + name + "(" + params->toStr() + ") {\n";
  for (auto BB : basicBlocks)
    str += BB->toStr();
  str += "}\n";
  return str;
}

string Program::getEntryPointLabel() const { return entryPointLabel; }
void Program::setEntryPointLabel(string label) { entryPointLabel = label; }
const vector<Function *> &Program::getFunctions() const { return functions; }
void Program::addFunction(Function *F) { functions.push_back(F); }
Function *Program::getCurrFunction() const { return functions.back(); }
string Program::toStr() const {
  string str;
  for (auto F : functions)
    str += F->toStr() + "\n";
  return str;
}

} // namespace L3
