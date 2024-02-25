#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <LA.h>

using namespace std;

namespace LA {

std::string Item::toStr() const { throw runtime_error("Item::toStr() not implemented"); }

Variable::Variable(const string &name, const VarType *type) : name(name), varType(type) {}
const string &Variable::getName() const { return name; }
const VarType *Variable::getVarType() const { return varType; }
string Variable::toStr() const { return name; }
string Variable::getPrefixedName() const { return "%" + name; }
string Variable::getDeclStr() const { return varType->toStr() + " " + getName(); }

Number::Number(int64_t val) : value(val) {}
int64_t Number::getValue() const { return value; }
string Number::toStr() const { return to_string(value); }

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

Int64Type *Int64Type::instance = new Int64Type();
Int64Type *Int64Type::getInstance() { return instance; }
string Int64Type::toStr() const { return "int64"; }
VarType::Type Int64Type::getType() const { return INT64; }

void ArrayType::increaseDim() { dim++; }
int64_t ArrayType::getDim() const { return dim; }
string ArrayType::toStr() const {
  string str = "int64";
  for (auto i = 0; i < dim; i++)
    str += "[]";
  return str;
}
VarType::Type ArrayType::getType() const { return ARRAY; }

TupleType *TupleType::instance = new TupleType();
TupleType *TupleType::getInstance() { return instance; }
string TupleType::toStr() const { return "tuple"; }
VarType::Type TupleType::getType() const { return TUPLE; }

CodeType *CodeType::instance = new CodeType();
CodeType *CodeType::getInstance() { return instance; }
string CodeType::toStr() const { return "code"; }
VarType::Type CodeType::getType() const { return CODE; }

VoidType *VoidType::instance = new VoidType();
VoidType *VoidType::getInstance() { return instance; }
string VoidType::toStr() const { return "void"; }
VarType::Type VoidType::getType() const { return VOID; }

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

void Parameters::addParamToHead(const Variable *param) { params.insert(params.begin(), param); }
const vector<const Variable *> &Parameters::getParams() const { return params; }
string Parameters::toStr() const {
  string str;

  for (auto param : params)
    str += param->getDeclStr() + ", ";

  return str.empty() ? str : str.substr(0, str.size() - 2);
}

Operator::Operator(ID id, string name) : id(id), name(std::move(name)) {}
Operator *Operator::getOperator(ID id) { return enumMap.at(id); }
string Operator::getName() const { return name; }
Operator::ID Operator::getID() const { return id; }
string Operator::toStr() const { return name; }
const unordered_map<Operator::ID, Operator *> Operator::enumMap = {
    {ID::ADD, new Operator(ID::ADD, "+")},
    {ID::SUB, new Operator(ID::SUB, "-")},
    {ID::MUL, new Operator(ID::MUL, "*")},
    {ID::AND, new Operator(ID::AND, "&")},
    {ID::LS, new Operator(ID::LS, "<<")},
    {ID::RS, new Operator(ID::RS, ">>")},
    {ID::LESS_THAN, new Operator(ID::LESS_THAN, "<")},
    {ID::LESS_EQUAL, new Operator(ID::LESS_EQUAL, "<=")},
    {ID::EQUAL, new Operator(ID::EQUAL, "=")},
    {ID::GREATER_EQUAL, new Operator(ID::GREATER_EQUAL, ">=")},
    {ID::GREATER_THAN, new Operator(ID::GREATER_THAN, ">")}};

RuntimeFunction::RuntimeFunction(string name) : name(std::move(name)) {}
RuntimeFunction *RuntimeFunction::getRuntimeFunction(ID id) { return enumMap.at(id); }
string RuntimeFunction::getName() const { return name; }
string RuntimeFunction::toStr() const { return name; }
string RuntimeFunction::getPrefixedName() const { return name; }
const unordered_map<RuntimeFunction::ID, RuntimeFunction *> RuntimeFunction::enumMap = {
    {ID::PRINT, new RuntimeFunction("print")}, {ID::INPUT, new RuntimeFunction("input")}};

UserFunction::UserFunction(string name) : name{std::move(name)} {}
string UserFunction::getName() const { return name; }
string UserFunction::toStr() const { return name; }
string UserFunction::getPrefixedName() const { return "@" + name; }

Label::Label(string name) : name{std::move(name)} {}
string Label::getName() const { return name; }
string Label::toStr() const { return name; }

/*
 *  Instructions.
 */
DeclarationInst::DeclarationInst(const Variable *var) : var(var) {}
const Variable *DeclarationInst::getVar() const { return var; }
string DeclarationInst::toStr() const { return var->getVarType()->toStr() + " " + var->toStr(); }
void DeclarationInst::accept(Visitor &visitor) const { visitor.visit(this); }

AssignInst::AssignInst(const Variable *lhs, const Value *rhs) : lhs(lhs), rhs(rhs) {}
const Variable *AssignInst::getLhs() const { return lhs; }
const Value *AssignInst::getRhs() const { return rhs; }
string AssignInst::toStr() const { return lhs->toStr() + " <- " + rhs->toStr(); }
void AssignInst::accept(Visitor &visitor) const { visitor.visit(this); }

OpInst::OpInst(const Variable *rst, const Value *lhs, const Operator *op, const Value *rhs)
    : rst(rst), lhs(lhs), op(op), rhs(rhs) {}
const Variable *OpInst::getRst() const { return rst; }
const Value *OpInst::getLhs() const { return lhs; }
const Operator *OpInst::getOp() const { return op; }
const Value *OpInst::getRhs() const { return rhs; }
string OpInst::toStr() const { return rst->toStr() + " <- " + lhs->toStr() + " " + op->toStr() + " " + rhs->toStr(); }
void OpInst::accept(Visitor &visitor) const { visitor.visit(this); }

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
const Variable *ArrayLenInst::getArray() const { return base; }
const Value *ArrayLenInst::getDimIndex() const { return dimIndex; }
string ArrayLenInst::toStr() const { return result->toStr() + " <- length " + base->toStr() + " " + dimIndex->toStr(); }
void ArrayLenInst::accept(Visitor &visitor) const { visitor.visit(this); }

TupleLenInst::TupleLenInst(const Variable *result, const Variable *base) : result{result}, base{base} {}
const Variable *TupleLenInst::getResult() const { return result; }
const Variable *TupleLenInst::getTuple() const { return base; }
string TupleLenInst::toStr() const { return result->toStr() + " <- length " + base->toStr(); }
void TupleLenInst::accept(Visitor &visitor) const { visitor.visit(this); }

NewArrayInst::NewArrayInst(const Variable *array, const vector<const Value *> &sizes) : array(array), sizes(sizes) {}
const Variable *NewArrayInst::getArray() const { return array; }
const vector<const Value *> &NewArrayInst::getSizes() const { return sizes; }
string NewArrayInst::toStr() const {
  string str = array->toStr() + " <- new Array(";
  for (auto size : sizes)
    str += size->toStr() + ", ";
  return str.substr(0, str.size() - 2) + ")";
}
void NewArrayInst::accept(Visitor &visitor) const { visitor.visit(this); }

NewTupleInst::NewTupleInst(const Variable *tuple, const Value *size) : tuple(tuple), size(size) {}
const Variable *NewTupleInst::getTuple() const { return tuple; }
const Value *NewTupleInst::getSize() const { return size; }
string NewTupleInst::toStr() const {
  string str = tuple->toStr() + " <- new Tuple(" + size->toStr() + ")";
  return str;
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
  return "br " + condition->toStr() + " " + (trueLabel == nullptr ? "<eliminated>" : trueLabel->toStr()) + " " +
         (falseLabel == nullptr ? "<eliminated>" : falseLabel->toStr());
}
void CondBranchInst::accept(Visitor &visitor) const { visitor.visit(this); }

CallInst::CallInst(const Name *callee, const Arguments *args) : callee{callee}, args{args} {}
const Name *CallInst::getCallee() const { return callee; }
const Arguments *CallInst::getArgs() const { return args; }
string CallInst::toStr() const { return callee->toStr() + "(" + args->toStr() + ")"; }
void CallInst::accept(Visitor &visitor) const { visitor.visit(this); }

CallAssignInst::CallAssignInst(const Variable *rst, const Name *callee, const Arguments *args)
    : rst{rst}, callee{callee}, args{args} {}
const Variable *CallAssignInst::getRst() const { return rst; }
const Name *CallAssignInst::getCallee() const { return callee; }
const Arguments *CallAssignInst::getArgs() const { return args; }
string CallAssignInst::toStr() const { return rst->toStr() + " <- " + callee->toStr() + "(" + args->toStr() + ")"; }
void CallAssignInst::accept(Visitor &visitor) const { visitor.visit(this); }

BasicBlock::BasicBlock() : label(nullptr), terminator(nullptr) {}
const std::vector<const Instruction *> &BasicBlock::getInstructions() const { return instructions; }
void BasicBlock::addInstruction(const Instruction *inst) { instructions.push_back(inst); }
const std::unordered_set<BasicBlock *> &BasicBlock::getPredecessors() const { return predecessors; }
const std::unordered_set<BasicBlock *> &BasicBlock::getSuccessors() const { return successors; }
void BasicBlock::addPredecessor(BasicBlock *predecessor) { predecessors.insert(predecessor); }
void BasicBlock::removePredecessor(BasicBlock *BB) { predecessors.erase(BB); }
void BasicBlock::addSuccessor(BasicBlock *successor) { successors.insert(successor); }
void BasicBlock::removeSuccessor(BasicBlock *BB) { successors.erase(BB); }
bool BasicBlock::empty() const { return instructions.empty() && !label && !terminator; }
string BasicBlock::toStr() const {
  string str = "  " + (label ? label->toStr() : ":<no-label>") + "\n";
  for (auto inst : instructions)
    str += "  " + inst->toStr() + "\n";
  str += "  " + (terminator ? terminator->toStr() : "<no-terminator>") + "\n";
  return str;
}
const Label *BasicBlock::getLabel() const { return label; }
void BasicBlock::setLabel(const Label *label) { this->label = label; }
const TerminatorInst *BasicBlock::getTerminator() const { return terminator; }
void BasicBlock::setTerminator(const TerminatorInst *terminator) { this->terminator = terminator; }

Function::Function() {
  this->basicBlocks.push_back(new BasicBlock());
  longestVarName = "var";
  varCounter = 0;
  longestLabelName = ":label";
  labelCounter = 0;
}
string Function::getName() const { return name; }
void Function::setName(const string &name) { this->name = name; }
const VarType *Function::getReturnType() const { return returnType; }
void Function::setReturnType(const VarType *type) { this->returnType = type; }
const Parameters *Function::getParams() const { return params; }
void Function::setParams(const Parameters *parameters) { this->params = parameters; }

void Function::defineVariable(const string &name, const VarType *type) {
  longestVarName = name.size() > longestVarName.size() ? name : longestVarName;
  if (variables.find(name) != variables.end())
    throw runtime_error("Variable " + name + " already defined");
  variables[name] = new Variable(name, type);
}
Variable *Function::getVariable(const string &name) {
  if (variables.find(name) == variables.end())
    throw runtime_error("Variable " + name + " not found");
  return variables[name];
}
bool Function::hasVariable(const string &name) const { return variables.find(name) != variables.end(); }
string Function::generateNewVariableName() { return longestVarName + "_global_" + to_string(varCounter++); }

const unordered_map<string, Variable *> &Function::getVariables() const { return variables; }
const unordered_map<string, Label *> &Function::getLabels() const { return labels; }
void Function::addInstruction(Instruction *inst) { basicBlocks.back()->addInstruction(inst); }
Label *Function::getLabel(const string &name) {
  if (labels.find(name) == labels.end())
    labels[name] = new Label(name);
  return labels[name];
}
const Label *Function::generateNewLabel() {
  string name = longestLabelName + "_global_" + to_string(labelCounter++);
  return getLabel(name);
}
string Function::generateNewLabelName() { return longestLabelName + "_global_" + to_string(labelCounter++); }

BasicBlock *Function::newBasicBlock() {
  if (basicBlocks.back()->empty())
    return basicBlocks.back();
  auto newBB = new BasicBlock();
  basicBlocks.push_back(newBB);
  return newBB;
}
BasicBlock *Function::getCurrBasicBlock() const { return basicBlocks.back(); }
const vector<BasicBlock *> &Function::getBasicBlocks() const { return basicBlocks; }

string Function::toStr() const {
  string str;
  str += returnType->toStr() + " " + name + "(" + params->toStr() + ") {\n";
  for (auto BB : basicBlocks)
    str += BB->toStr() + "\n";
  str += "}\n";
  return str;
}
Program::Program() {}
const vector<Function *> &Program::getFunctions() const { return functions; }
void Program::addFunction(Function *F) { functions.push_back(F); }
Function *Program::getCurrFunction() const { return functions.back(); }

void Program::addInstructionToCurrFunc(Instruction *inst, int64_t lineno) {
  inst->lineno = lineno;
  getCurrFunction()->addInstruction(inst);
}
void Program::declareVariableInCurrFunc(const string &name, const VarType *type) {
  getCurrFunction()->defineVariable(name, type);
}
Variable *Program::getVariableInCurrFunc(const string &name) const { return getCurrFunction()->getVariable(name); }
Label *Program::getLabelInCurrFunc(const string &name) { return getCurrFunction()->getLabel(name); }
bool Program::currFuncHasVariable(const string &name) const { return getCurrFunction()->hasVariable(name); }
BasicBlock *Program::newBasicBlock() const { return getCurrFunction()->newBasicBlock(); }

string Program::toStr() const {
  string str;
  for (auto F : functions)
    str += F->toStr() + "\n";
  return str;
}
} // namespace LA
