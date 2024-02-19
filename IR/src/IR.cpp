#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
using namespace std;

#include <IR.h>

namespace IR {

std::string Item::toStr() const { throw runtime_error("Item::toStr() not implemented"); }
void Item::accept(Visitor &visitor) const { throw runtime_error("Item::accept() not implemented"); }

Variable::Variable(const string &name, const Type *type) : name(name), type(type) {}
const string &Variable::getName() const { return name; }
const Type *Variable::getType() const { return type; }
string Variable::toStr() const { return name; }
void Variable::accept(Visitor &visitor) const { visitor.visit(this); }

Number::Number(int64_t val) : value(val) {}
int64_t Number::getValue() const { return value; }
string Number::toStr() const { return to_string(value); }
void Number::accept(Visitor &visitor) const { visitor.visit(this); }

MemoryLocation::MemoryLocation(const Variable *base) : base(base) {}
const Variable *MemoryLocation::getBase() const { return base; }
void MemoryLocation::addIndex(const Value *index) { indices.push_back(index); }
const vector<const Value *> &MemoryLocation::getIndices() const { return indices; }
string MemoryLocation::toStr() const {
  string str = base->toStr();
  for (auto index : indices)
    str += "[" + index->toStr() + "]";
  return str;
}
void MemoryLocation::accept(Visitor &visitor) const { visitor.visit(this); }

Int64Type *Int64Type::instance = new Int64Type();
Int64Type *Int64Type::getInstance() { return instance; }
string Int64Type::toStr() const { return "int64"; }
void Int64Type::accept(Visitor &visitor) const { visitor.visit(this); }

void ArrayType::increaseDim() { dim++; }
int64_t ArrayType::getDim() const { return dim; }
void ArrayType::setSizes(const vector<const Value *> &sizes) { this->sizes = sizes; }
const vector<const Value *> &ArrayType::getSizes() const { return sizes; }
string ArrayType::toStr() const {
  string str = "int64";
  for (auto i = 0; i < dim; i++)
    str += "[]";
  return str;
}
void ArrayType::accept(Visitor &visitor) const { visitor.visit(this); }

void TupleType::setSize(const Value *size) { this->size = size; }
const Value *TupleType::getSize() const { return size; }
string TupleType::toStr() const { return "tuple"; }
void TupleType::accept(Visitor &visitor) const { visitor.visit(this); }

CodeType *CodeType::instance = new CodeType();
CodeType *CodeType::getInstance() { return instance; }
string CodeType::toStr() const { return "code"; }
void CodeType::accept(Visitor &visitor) const { visitor.visit(this); }

VoidType *VoidType::instance = new VoidType();
VoidType *VoidType::getInstance() { return instance; }
string VoidType::toStr() const { return "void"; }
void VoidType::accept(Visitor &visitor) const { visitor.visit(this); }

LeftParen *LeftParen::instance = new LeftParen();
LeftParen *LeftParen::getInstance() { return instance; }

RightParen *RightParen::instance = new RightParen();
RightParen *RightParen::getInstance() { return instance; }

void Arguments::addArgToHead(const Value *arg) { args.insert(args.begin(), arg); }
void Arguments::addArgToTail(const Value *arg) { args.push_back(arg); }
const vector<const Value *> &Arguments::getArgs() const { return args; }
string Arguments::toStr() const {
  string str;

  for (auto arg : args)
    str += arg->toStr() + ", ";

  return str.empty() ? str : str.substr(0, str.size() - 2);
}
void Arguments::accept(Visitor &visitor) const { visitor.visit(this); }

void Parameters::addParamToHead(const Variable *param) { params.insert(params.begin(), param); }
const vector<const Variable *> &Parameters::getParams() const { return params; }
string Parameters::toStr() const {
  string str;

  for (auto param : params)
    str += param->getType()->toStr() + " " + param->toStr() + ", ";

  return str.empty() ? str : str.substr(0, str.size() - 2);
}
void Parameters::accept(Visitor &visitor) const { visitor.visit(this); }

CompareOp::CompareOp(ID id, string name) : id(id), name{std::move(name)} {}
CompareOp *CompareOp::getCompareOp(ID id) { return enumMap.at(id); }
string CompareOp::getName() const { return name; }
CompareOp::ID CompareOp::getID() const { return id; }
string CompareOp::toStr() const { return name; }
void CompareOp::accept(Visitor &visitor) const { visitor.visit(this); }
const unordered_map<CompareOp::ID, CompareOp *> CompareOp::enumMap = {
    {ID::LESS_THAN, new CompareOp(ID::LESS_THAN, "<")},
    {ID::LESS_EQUAL, new CompareOp(ID::LESS_EQUAL, "<=")},
    {ID::EQUAL, new CompareOp(ID::EQUAL, "=")},
    {ID::GREATER_EQUAL, new CompareOp(ID::GREATER_EQUAL, ">=")},
    {ID::GREATER_THAN, new CompareOp(ID::GREATER_THAN, ">")}};

ArithOp::ArithOp(ID id, string name) : id(id), name(std::move(name)) {}
ArithOp *ArithOp::getArithOp(ID id) { return enumMap.at(id); }
string ArithOp::getName() const { return name; }
ArithOp::ID ArithOp::getID() const { return id; }
string ArithOp::toStr() const { return name; }
void ArithOp::accept(Visitor &visitor) const { visitor.visit(this); }
const unordered_map<ArithOp::ID, ArithOp *> ArithOp::enumMap = {
    {ID::ADD, new ArithOp(ID::ADD, "+")}, {ID::SUB, new ArithOp(ID::SUB, "-")}, {ID::MUL, new ArithOp(ID::MUL, "*")},
    {ID::AND, new ArithOp(ID::AND, "&")}, {ID::LS, new ArithOp(ID::LS, "<<")},  {ID::RS, new ArithOp(ID::RS, ">>")}};

RuntimeFunction::RuntimeFunction(string name) : name(std::move(name)) {}
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

FunctionName::FunctionName(string name) : name{std::move(name)} {}
string FunctionName::getName() const { return name; }
string FunctionName::toStr() const { return name; }
void FunctionName::accept(Visitor &visitor) const { visitor.visit(this); }

Label::Label(string name) : name{std::move(name)} {}
string Label::getName() const { return name; }
string Label::toStr() const { return name; }
void Label::accept(Visitor &visitor) const { visitor.visit(this); }

/*
 *  Instructions.
 */
DeclarationInst::DeclarationInst(const Variable *var) : var(var) {}
const Variable *DeclarationInst::getVar() const { return var; }
string DeclarationInst::toStr() const { return var->getType()->toStr() + " " + var->toStr(); }
void DeclarationInst::accept(Visitor &visitor) const { visitor.visit(this); }

AssignInst::AssignInst(const Variable *lhs, const Item *rhs) : lhs(lhs), rhs(rhs) {}
const Variable *AssignInst::getLhs() const { return lhs; }
const Item *AssignInst::getRhs() const { return rhs; }
string AssignInst::toStr() const { return lhs->toStr() + " <- " + rhs->toStr(); }
void AssignInst::accept(Visitor &visitor) const { visitor.visit(this); }

ArithInst::ArithInst(const Variable *rst, const Value *lhs, const ArithOp *op, const Value *rhs)
    : rst(rst), lhs(lhs), op(op), rhs(rhs) {}
const Variable *ArithInst::getRst() const { return rst; }
const Value *ArithInst::getLhs() const { return lhs; }
const ArithOp *ArithInst::getOp() const { return op; }
const Value *ArithInst::getRhs() const { return rhs; }
string ArithInst::toStr() const {
  return rst->toStr() + " <- " + lhs->toStr() + " " + op->toStr() + " " + rhs->toStr();
}
void ArithInst::accept(Visitor &visitor) const { visitor.visit(this); }

CompareInst::CompareInst(const Variable *rst, const Value *lhs, const CompareOp *op, const Value *rhs)
    : rst{rst}, lhs{lhs}, op{op}, rhs{rhs} {}
const Variable *CompareInst::getRst() const { return rst; }
const CompareOp *CompareInst::getOp() const { return op; }
const Value *CompareInst::getLhs() const { return lhs; }
const Value *CompareInst::getRhs() const { return rhs; }
string CompareInst::toStr() const {
  return rst->toStr() + " <- " + lhs->toStr() + " " + op->toStr() + " " + rhs->toStr();
}
void CompareInst::accept(Visitor &visitor) const { visitor.visit(this); }

LoadInst::LoadInst(const Variable *target, const MemoryLocation *memLoc) : target(target), memLoc(memLoc) {}
const Variable *LoadInst::getTarget() const { return target; }
const MemoryLocation *LoadInst::getMemLoc() const { return memLoc; }
string LoadInst::toStr() const {
  string str = target->toStr() + " <- ";
  str += memLoc->getBase()->toStr();
  for (auto index : memLoc->getIndices())
    str += "[" + index->toStr() + "]";
  return str;
}
void LoadInst::accept(Visitor &visitor) const { visitor.visit(this); }

StoreInst::StoreInst(const MemoryLocation *memLoc, const Value *source) : memLoc(memLoc), source(source) {}
const MemoryLocation *StoreInst::getMemLoc() const { return memLoc; }
const Value *StoreInst::getSource() const { return source; }
string StoreInst::toStr() const {
  string str = memLoc->getBase()->toStr();
  for (auto index : memLoc->getIndices())
    str += "[" + index->toStr() + "]";
  str += " <- " + source->toStr();
  return str;
}
void StoreInst::accept(Visitor &visitor) const { visitor.visit(this); }

ArrayLenInst::ArrayLenInst(const Variable *result, const Variable *base, const Value *dimIndex)
    : result{result}, base{base}, dimIndex{dimIndex} {}
const Variable *ArrayLenInst::getResult() const { return result; }
const Variable *ArrayLenInst::getBase() const { return base; }
const Value *ArrayLenInst::getDimIndex() const { return dimIndex; }
string ArrayLenInst::toStr() const { return result->toStr() + " <- length " + base->toStr() + " " + dimIndex->toStr(); }
void ArrayLenInst::accept(Visitor &visitor) const { visitor.visit(this); }

TupleLenInst::TupleLenInst(const Variable *result, const Variable *base) : result{result}, base{base} {}
const Variable *TupleLenInst::getResult() const { return result; }
const Variable *TupleLenInst::getBase() const { return base; }
string TupleLenInst::toStr() const { return result->toStr() + " <- length " + base->toStr(); }
void TupleLenInst::accept(Visitor &visitor) const { visitor.visit(this); }

NewArrayInst::NewArrayInst(const Variable *array) : array{array} {}
const Variable *NewArrayInst::getArray() const { return array; }
string NewArrayInst::toStr() const {
  string str = array->toStr() + " <- new Array(";
  auto sizes = ((ArrayType *)array->getType())->getSizes();
  for (auto size : sizes)
    str += size->toStr() + ", ";
  return str.substr(0, str.size() - 2) + ")";
}
void NewArrayInst::accept(Visitor &visitor) const { visitor.visit(this); }

NewTupleInst::NewTupleInst(const Variable *tuple) : tuple{tuple} {}
const Variable *NewTupleInst::getTuple() const { return tuple; }
string NewTupleInst::toStr() const {
  string str = tuple->toStr() + " <- new Tuple(";
  auto size = ((TupleType *)tuple->getType())->getSize();
  str += size->toStr();
  return str + ")";
}
void NewTupleInst::accept(Visitor &visitor) const { visitor.visit(this); }

string RetInst::toStr() const { return "return"; }
void RetInst::accept(Visitor &visitor) const { visitor.visit(this); }

RetValueInst::RetValueInst(const Value *value) : value{value} {}
const Value *RetValueInst::getValue() const { return value; }
string RetValueInst::toStr() const { return "return " + value->toStr(); }
void RetValueInst::accept(Visitor &visitor) const { visitor.visit(this); }

LabelInst::LabelInst(const Label *label) : label{label} {}
const Label *LabelInst::getLabel() const { return label; }
string LabelInst::toStr() const { return label->toStr(); }
void LabelInst::accept(Visitor &visitor) const { visitor.visit(this); }

BranchInst::BranchInst(const Label *label) : label{label} {}
const Label *BranchInst::getLabel() const { return label; }
string BranchInst::toStr() const { return "br " + label->toStr(); }
void BranchInst::accept(Visitor &visitor) const { visitor.visit(this); }

CondBranchInst::CondBranchInst(const Value *condition, const Label *trueLabel, const Label *falseLabel)
    : condition{condition}, trueLabel{trueLabel}, falseLabel{falseLabel} {}
const Value *CondBranchInst::getCondition() const { return condition; }
const Label *CondBranchInst::getTrueLabel() const { return trueLabel; }
const Label *CondBranchInst::getFalseLabel() const { return falseLabel; }
string CondBranchInst::toStr() const {
  return "br " + condition->toStr() + " " + trueLabel->toStr() + " " + falseLabel->toStr();
}
void CondBranchInst::accept(Visitor &visitor) const { visitor.visit(this); }

CallInst::CallInst(const Item *callee, const Arguments *args) : callee{callee}, args{args} {}
const Item *CallInst::getCallee() const { return callee; }
const Arguments *CallInst::getArgs() const { return args; }
string CallInst::toStr() const { return "call " + callee->toStr() + "(" + args->toStr() + ")"; }
void CallInst::accept(Visitor &visitor) const { visitor.visit(this); }

CallAssignInst::CallAssignInst(const Variable *rst, const Item *callee, const Arguments *args)
    : rst{rst}, callee{callee}, args{args} {}
const Variable *CallAssignInst::getRst() const { return rst; }
const Item *CallAssignInst::getCallee() const { return callee; }
const Arguments *CallAssignInst::getArgs() const { return args; }
string CallAssignInst::toStr() const {
  return rst->toStr() + " <- call " + callee->toStr() + "(" + args->toStr() + ")";
}
void CallAssignInst::accept(Visitor &visitor) const { visitor.visit(this); }

const std::vector<const Instruction *> &BasicBlock::getInstructions() const { return instructions; }
void BasicBlock::addInstruction(const Instruction *inst) { instructions.push_back(inst); }
const std::unordered_set<BasicBlock *> &BasicBlock::getPredecessors() const { return predecessors; }
const std::unordered_set<BasicBlock *> &BasicBlock::getSuccessors() const { return successors; }
void BasicBlock::addPredecessor(BasicBlock *predecessor) { predecessors.insert(predecessor); }
void BasicBlock::removePredecessor(BasicBlock *BB) { predecessors.erase(BB); }
void BasicBlock::addSuccessor(BasicBlock *successor) { successors.insert(successor); }
void BasicBlock::removeSuccessor(BasicBlock *BB) { successors.erase(BB); }
const Instruction *BasicBlock::getFirstInstruction() const { return instructions.front(); }
const Instruction *BasicBlock::getTerminator() const { return instructions.back(); }
bool BasicBlock::empty() const { return instructions.empty(); }
string BasicBlock::toStr() const {
  string str;
  for (auto inst : instructions)
    str += "  " + inst->toStr() + "\n";
  return str;
}

Function::Function() { this->basicBlocks.push_back(new BasicBlock()); }
string Function::getName() const { return name; }
void Function::setName(const string &name) { this->name = name; }
const Type *Function::getReturnType() const { return returnType; }
void Function::setReturnType(const Type *type) { this->returnType = type; }
const Parameters *Function::getParams() const { return params; }
void Function::setParams(const Parameters *parameters) { this->params = parameters; }
void Function::defineVariable(const string &name, const Type *type) {
  if (variables.find(name) != variables.end())
    throw runtime_error("Variable " + name + " already defined");
  variables[name] = new Variable(name, type);
}
Variable *Function::getVariable(const string &name) {
  if (variables.find(name) == variables.end())
    throw runtime_error("Variable " + name + " not found");
  return variables[name];
}
const unordered_map<string, Variable *> &Function::getVariables() const { return variables; }
const unordered_map<string, Label *> &Function::getLabels() const { return labels; }
void Function::addInstruction(Instruction *inst) { basicBlocks.back()->addInstruction(inst); }
Label *Function::getLabel(const string &name) {
  if (labels.find(name) == labels.end())
    labels[name] = new Label(name);
  return labels[name];
}

void Function::newBasicBlock() {
  if (basicBlocks.back()->empty())
    return;
  basicBlocks.push_back(new BasicBlock());
}
const vector<BasicBlock *> &Function::getBasicBlocks() const { return basicBlocks; }

string Function::toStr() const {
  string str;
  str += "define " + returnType->toStr() + " " + name + "(" + params->toStr() + ") {\n";
  for (auto BB : basicBlocks)
    str += BB->toStr() + "\n";
  str += "}\n";
  return str;
}

const vector<Function *> &Program::getFunctions() const { return functions; }
void Program::addFunction(Function *F) { functions.push_back(F); }
Function *Program::getCurrFunction() const { return functions.back(); }
void Program::addInstruction(Instruction *inst) { getCurrFunction()->addInstruction(inst); }
void Program::defineVariable(const string &name, const Type *type) { getCurrFunction()->defineVariable(name, type); }
Variable *Program::getVariable(const string &name) const { return getCurrFunction()->getVariable(name); }
Label *Program::getLabel(const string &name) const { return getCurrFunction()->getLabel(name); }
void Program::newBasicBlock() const { getCurrFunction()->newBasicBlock(); }
string Program::toStr() const {
  string str;
  for (auto F : functions)
    str += F->toStr() + "\n";
  return str;
}
} // namespace IR
