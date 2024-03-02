#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <LB.h>

using namespace std;

namespace LB {

std::string Item::toStr() const { throw runtime_error("Item::toStr() not implemented"); }
std::string Item::getLAToken() const { throw runtime_error("Item::getLAToken() not implemented"); }

Variable::Variable(const string &rawName, const string &globName, VarType *type)
    : rawName(rawName), globName(globName), varType(type) {}
string Variable::toStr() const { return rawName; }
string Variable::getLAToken() const { return globName; }

string VariableList::toStr() const {
  string str;
  for (auto var : variables)
    str += var->toStr() + ", ";
  return str.substr(0, str.size() - 2);
}
string VariableList::getLAToken() const {
  string str;
  for (auto var : variables)
    str += var->getLAToken() + ", ";
  return str.substr(0, str.size() - 2);
}

Number::Number(int64_t val) : value(val) {}
int64_t Number::getValue() const { return value; }
string Number::toStr() const { return to_string(value); }
string Number::getLAToken() const { return toStr(); }

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
string MemoryLocation::getLAToken() const {
  string str = base->getLAToken();
  for (auto index : indices)
    str += "[" + index->getLAToken() + "]";
  return str;
}

Int64Type *Int64Type::instance = new Int64Type();
Int64Type *Int64Type::getInstance() { return instance; }
string Int64Type::toStr() const { return "int64"; }
string Int64Type::getLAToken() const { return toStr(); }
VarType::Type Int64Type::getType() const { return INT64; }

void ArrayType::increaseDim() { dim++; }
int64_t ArrayType::getDim() const { return dim; }
string ArrayType::toStr() const {
  string str = "int64";
  for (auto i = 0; i < dim; i++)
    str += "[]";
  return str;
}
string ArrayType::getLAToken() const { return toStr(); }
VarType::Type ArrayType::getType() const { return ARRAY; }

TupleType *TupleType::instance = new TupleType();
TupleType *TupleType::getInstance() { return instance; }
string TupleType::toStr() const { return "tuple"; }
string TupleType::getLAToken() const { return toStr(); }
VarType::Type TupleType::getType() const { return TUPLE; }

CodeType *CodeType::instance = new CodeType();
CodeType *CodeType::getInstance() { return instance; }
string CodeType::toStr() const { return "code"; }
string CodeType::getLAToken() const { return toStr(); }
VarType::Type CodeType::getType() const { return CODE; }

VoidType *VoidType::instance = new VoidType();
VoidType *VoidType::getInstance() { return instance; }
string VoidType::toStr() const { return "void"; }
string VoidType::getLAToken() const { return toStr(); }
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
string Arguments::getLAToken() const {
  string str;
  for (auto arg : args)
    str += arg->getLAToken() + ", ";
  return str.empty() ? str : str.substr(0, str.size() - 2);
}

void Parameters::addParamToHead(const Variable *param) { params.insert(params.begin(), param); }
void Parameters::addParamToTail(const Variable *param) { params.push_back(param); }
const vector<const Variable *> &Parameters::getParams() const { return params; }
string Parameters::toStr() const {
  string str;
  for (auto param : params)
    str += param->varType->toStr() + " " + param->toStr() + ", ";
  return str.empty() ? str : str.substr(0, str.size() - 2);
}
string Parameters::getLAToken() const {
  string str;
  for (auto param : params)
    str += param->varType->getLAToken() + " " + param->getLAToken() + ", ";
  return str.empty() ? str : str.substr(0, str.size() - 2);
}

CmpOp::CmpOp(ID id, string name) : id(id), name(std::move(name)) {}
CmpOp *CmpOp::getOp(ID id) { return enumMap.at(id); }
string CmpOp::getName() const { return name; }
CmpOp::ID CmpOp::getID() const { return id; }
string CmpOp::toStr() const { return name; }
string CmpOp::getLAToken() const { return toStr(); }
const unordered_map<CmpOp::ID, CmpOp *> CmpOp::enumMap = {
    {ID::LESS_THAN, new CmpOp(ID::LESS_THAN, "<")},
    {ID::LESS_EQUAL, new CmpOp(ID::LESS_EQUAL, "<=")},
    {ID::EQUAL, new CmpOp(ID::EQUAL, "=")},
    {ID::GREATER_EQUAL, new CmpOp(ID::GREATER_EQUAL, ">=")},
    {ID::GREATER_THAN, new CmpOp(ID::GREATER_THAN, ">")},
};

Op::Op(ID id, string name) : id(id), name(std::move(name)) {}
Op *Op::getOp(ID id) { return enumMap.at(id); }
string Op::getName() const { return name; }
Op::ID Op::getID() const { return id; }
string Op::toStr() const { return name; }
string Op::getLAToken() const { return toStr(); }
const unordered_map<Op::ID, Op *> Op::enumMap = {
    {ID::ADD, new Op(ID::ADD, "+")}, {ID::SUB, new Op(ID::SUB, "-")}, {ID::MUL, new Op(ID::MUL, "*")},
    {ID::AND, new Op(ID::AND, "&")}, {ID::LS, new Op(ID::LS, "<<")},  {ID::RS, new Op(ID::RS, ">>")},
};

RuntimeFunction::RuntimeFunction(string name) : name(std::move(name)) {}
RuntimeFunction *RuntimeFunction::getRuntimeFunction(ID id) { return enumMap.at(id); }
string RuntimeFunction::getName() const { return name; }
string RuntimeFunction::toStr() const { return name; }
string RuntimeFunction::getLAToken() const { return toStr(); }
const unordered_map<RuntimeFunction::ID, RuntimeFunction *> RuntimeFunction::enumMap = {
    {ID::PRINT, new RuntimeFunction("print")}, {ID::INPUT, new RuntimeFunction("input")}};

UserFunction::UserFunction(string name) : name{std::move(name)} {}
string UserFunction::getName() const { return name; }
string UserFunction::toStr() const { return name; }
string UserFunction::getLAToken() const { return toStr(); }

Label::Label(string name) : name{std::move(name)} {}
string Label::getName() const { return name; }
string Label::toStr() const { return name; }
string Label::getLAToken() const { return toStr(); }

/*
 *  Instructions.
 */
DeclarationInst::DeclarationInst(VariableList *declaredVars) : declaredVars(declaredVars) {}
string DeclarationInst::toStr() const {
  auto type = declaredVars->variables[0]->varType;
  return type->toStr() + " " + declaredVars->toStr();
}
void DeclarationInst::accept(Visitor &visitor) const { visitor.visit(this); }

AssignInst::AssignInst(const Variable *lhs, const Value *rhs) : lhs(lhs), rhs(rhs) {}
const Variable *AssignInst::getLhs() const { return lhs; }
const Value *AssignInst::getRhs() const { return rhs; }
string AssignInst::toStr() const { return lhs->toStr() + " <- " + rhs->toStr(); }
void AssignInst::accept(Visitor &visitor) const { visitor.visit(this); }

CmpInst::CmpInst(Variable *rst, Value *lhs, CmpOp *op, Value *rhs) : rst(rst), lhs(lhs), op(op), rhs(rhs) {}
string CmpInst::toStr() const { return rst->toStr() + " <- " + lhs->toStr() + " " + op->toStr() + " " + rhs->toStr(); }
void CmpInst::accept(Visitor &visitor) const { visitor.visit(this); }

OpInst::OpInst(const Variable *rst, const Value *lhs, const Op *op, const Value *rhs)
    : rst(rst), lhs(lhs), op(op), rhs(rhs) {}
const Variable *OpInst::getRst() const { return rst; }
const Value *OpInst::getLhs() const { return lhs; }
const Op *OpInst::getOp() const { return op; }
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

IfInst::IfInst(Value *lhs, CmpOp *op, Value *rhs, Label *trueLabel, Label *falseLabel)
    : lhs(lhs), op(op), rhs(rhs), trueLabel(trueLabel), falseLabel(falseLabel) {}
string IfInst::toStr() const {
  return "if (" + lhs->toStr() + " " + op->toStr() + " " + rhs->toStr() + ") " + trueLabel->toStr() + " " +
         falseLabel->toStr();
}
void IfInst::accept(Visitor &visitor) const { visitor.visit(this); }

WhileInst::WhileInst(Value *lhs, CmpOp *op, Value *rhs, Label *bodyLabel, Label *exitLabel)
    : lhs(lhs), op(op), rhs(rhs), bodyLabel(bodyLabel), exitLabel(exitLabel) {}
string WhileInst::toStr() const {
  return "while (" + lhs->toStr() + " " + op->toStr() + " " + rhs->toStr() + ") " + bodyLabel->toStr() + " " +
         exitLabel->toStr();
}
void WhileInst::accept(Visitor &visitor) const { visitor.visit(this); }

string ContinueInst::toStr() const { return "continue"; }
void ContinueInst::accept(Visitor &visitor) const { visitor.visit(this); }

string BreakInst::toStr() const { return "break"; }
void BreakInst::accept(Visitor &visitor) const { visitor.visit(this); }

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

GotoInst::GotoInst(Label *label) : label{label} {}
string GotoInst::toStr() const { return "br " + label->toStr(); }
void GotoInst::accept(Visitor &visitor) const { visitor.visit(this); }

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

Scope::Scope(Scope *parent) {
  this->parent = parent;
  currType = nullptr;
}
Scope *Scope::getParent() const { return parent; }
Variable *Scope::declareVariable(const string &rawName, const string &globName, VarType *type) {
  if (variables.find(rawName) != variables.end())
    throw runtime_error("Variable " + rawName + " already defined");
  auto var = new Variable(rawName, globName, type);
  variables[rawName] = var;
  return var;
}
Variable *Scope::getVariable(const string &rawName) {
  if (variables.find(rawName) != variables.end())
    return variables[rawName];
  return nullptr;
}
void Scope::setType(VarType *type) { currType = type; }

Function::Function() {
  currScope = new Scope(nullptr);
  varPrefix = "var";
  varCounter = 0;
  longestLabelName = ":label";
  labelCounter = 0;
  params = new Parameters();
}
vector<Instruction *> &Function::getInstructions() { return instructions; }
string Function::getName() const { return name; }
void Function::setName(const string &name) { this->name = name; }
const VarType *Function::getReturnType() const { return returnType; }
void Function::setReturnType(const VarType *type) { this->returnType = type; }
const Parameters *Function::getParams() const { return params; }

Variable *Function::declareVariable(const string &rawName) {
  string globName = getNewGlobVarName();
  return currScope->declareVariable(rawName, globName, currType);
}
void Function::addParam(const string &name, VarType *type) {
  auto param = currScope->declareVariable(name, name, type);
  params->addParamToTail(param);
}
Variable *Function::getVariable(const string &rawName) {
  auto curr = currScope;
  while (curr != nullptr) {
    auto var = curr->getVariable(rawName);
    if (var)
      return var;
    curr = curr->getParent();
  }
  return nullptr;
}
string Function::getNewGlobVarName() { return varPrefix + "_" + to_string(varCounter++); }

const unordered_map<string, Label *> &Function::getLabels() const { return labels; }
void Function::addInstruction(Instruction *inst, int64_t lineno) {
  inst->lineno = lineno;
  instructions.push_back(inst);
}
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

void Function::enterScope() { currScope = new Scope(currScope); }
void Function::exitScope() {
  if (currScope->getParent() == nullptr)
    throw runtime_error("Cannot exit the outmost scope");
  auto oldScope = currScope;
  currScope = currScope->getParent();
  delete oldScope;
}
void Function::setCurrType(VarType *type) { currType = type; };

string Function::toStr() const {
  string str;
  str += returnType->toStr() + " " + name + "(" + params->toStr() + ") {\n";
  for (auto I : instructions)
    str += "  " + I->toStr() + "\n";
  str += "}\n";
  return str;
}

Program::Program() {}
const vector<Function *> &Program::getFunctions() const { return functions; }
void Program::addFunction(Function *F) { functions.push_back(F); }
Function *Program::getCurrFunction() const { return functions.back(); }

string Program::toStr() const {
  string str;
  for (auto F : functions)
    str += F->toStr() + "\n";
  return str;
}
} // namespace LB
