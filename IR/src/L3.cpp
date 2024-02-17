#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>
using namespace std;

#include <L3.h>

namespace L3 {

std::string Item::toStr() const { throw runtime_error("Item::toStr() not implemented"); }
void Item::accept(Visitor &visitor) const { throw runtime_error("Item::accept() not implemented"); }

Variable::Variable(string name) : name(std::move(name)) {}
string Variable::toStr() const { return name; }
void Variable::accept(Visitor &visitor) const { visitor.visit(this); }
std::string Variable::getName() const { return name; }

Number::Number(int64_t val) : val(val) {}
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

  return str.empty() ? str : str.substr(0, str.size() - 2);
}
void Arguments::accept(Visitor &visitor) const { visitor.visit(this); }

void Parameters::addParamToHead(const Variable *param) { params.insert(params.begin(), param); }
const vector<const Variable *> &Parameters::getParams() const { return params; }
string Parameters::toStr() const {
  string str;

  for (auto param : params)
    str += param->toStr() + ", ";

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

ArithOp::ArithOp(ID id, string name) : id(id), name{std::move(name)} {}
ArithOp *ArithOp::getArithOp(ID id) { return enumMap.at(id); }
string ArithOp::getName() const { return name; }
ArithOp::ID ArithOp::getID() const { return id; }
string ArithOp::toStr() const { return name; }
void ArithOp::accept(Visitor &visitor) const { visitor.visit(this); }
const unordered_map<ArithOp::ID, ArithOp *> ArithOp::enumMap = {
    {ID::ADD, new ArithOp(ID::ADD, "+")}, {ID::SUB, new ArithOp(ID::SUB, "-")}, {ID::MUL, new ArithOp(ID::MUL, "*")},
    {ID::AND, new ArithOp(ID::AND, "&")}, {ID::LS, new ArithOp(ID::LS, "<<")},  {ID::RS, new ArithOp(ID::RS, ">>")}};

RuntimeFunction::RuntimeFunction(string name) : name{std::move(name)} {}
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
void Instruction::setContext(const Context *context) { this->cxt = context; }
const Context *Instruction::getContext() const { return cxt; }

AssignInst::AssignInst(const Variable *lhs, const Item *rhs) : lhs{lhs}, rhs{rhs} {}
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

LoadInst::LoadInst(const Variable *val, const Variable *addr) : val{val}, addr{addr} {}
const Variable *LoadInst::getVal() const { return val; }
const Variable *LoadInst::getAddr() const { return addr; }
string LoadInst::toStr() const { return val->toStr() + " <- load " + addr->toStr(); }
void LoadInst::accept(Visitor &visitor) const { visitor.visit(this); }

StoreInst::StoreInst(const Variable *addr, const Value *val) : addr{addr}, val{val} {}
const Variable *StoreInst::getAddr() const { return addr; }
const Value *StoreInst::getVal() const { return val; }
string StoreInst::toStr() const { return "store " + addr->toStr() + " <- " + val->toStr(); }
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

CondBranchInst::CondBranchInst(const Value *condition, const Label *label) : condition{condition}, label{label} {}
const Value *CondBranchInst::getCondition() const { return condition; }
const Label *CondBranchInst::getLabel() const { return label; }
string CondBranchInst::toStr() const { return "br " + condition->toStr() + " " + label->toStr(); }
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
    str += inst->toStr() + "\n";
  return str;
}

const vector<const Instruction *> &Context::getInstructions() const { return instructions; }
void Context::addInstruction(const Instruction *inst) { instructions.push_back(inst); }

Function::Function(string name) : name{std::move(name)} { this->basicBlocks.push_back(new BasicBlock()); }
string Function::getName() const { return name; }
const Parameters *Function::getParams() const { return params; }
void Function::setParams(const Parameters *parameters) { this->params = parameters; }
const Variable *Function::getVariable(const string &name) {
  if (variables.find(name) == variables.end())
    variables[name] = new Variable(name);
  return variables[name];
}
const unordered_map<string, Label *> &Function::getLabels() const { return labels; }
void Function::addInstruction(Instruction *inst) { basicBlocks.back()->addInstruction(inst); }
const Label *Function::getLabel(const string &name) {
  if (labels.find(name) == labels.end())
    labels[name] = new Label(name);
  return labels[name];
}
void Function::newBasicBlock() {
  if (basicBlocks.back()->empty())
    return;
  basicBlocks.push_back(new BasicBlock());
}
void Function::newLinkedBasicBlock() {
  if (basicBlocks.back()->empty())
    throw runtime_error("cannot link an empty basic block");
  auto newBB = new BasicBlock();
  basicBlocks.back()->addSuccessor(newBB);
  newBB->addPredecessor(basicBlocks.back());
  basicBlocks.push_back(newBB);
}
const vector<BasicBlock *> &Function::getBasicBlocks() const { return basicBlocks; }
string Function::toStr() const {
  string str;
  str += "define " + name + "(" + params->toStr() + ") {\n";
  for (auto BB : basicBlocks)
    str += "  " + BB->toStr() + "\n";
  str += "}\n";
  return str;
}

Program::Program() { this->currContext = new Context(); }
const vector<Function *> &Program::getFunctions() const { return functions; }
void Program::addFunction(Function *F) { functions.push_back(F); }
Function *Program::getCurrFunction() const { return functions.back(); }
void Program::addInstruction(Instruction *inst) {
  inst->setContext(currContext);
  if (currContext)
    currContext->addInstruction(inst);
  getCurrFunction()->addInstruction(inst);
}
void Program::newContext() { this->currContext = new Context(); }
void Program::closeContext() { this->currContext = nullptr; }
const Variable *Program::getVariable(const string &name) const { return getCurrFunction()->getVariable(name); }
const Label *Program::getLabel(const string &name) const { return getCurrFunction()->getLabel(name); }
void Program::newBasicBlock() const { getCurrFunction()->newBasicBlock(); }
void Program::newLinkedBasicBlock() const { getCurrFunction()->newLinkedBasicBlock(); }
string Program::toStr() const {
  string str;
  for (auto F : functions)
    str += F->toStr() + "\n";
  return str;
}
} // namespace L3
