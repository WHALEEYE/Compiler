#include <L2.h>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace L2 {

Symbol::Symbol(std::string name) : name{name} {}
const std::string Symbol::getName() const { return name; }

Register::Register(std::string name, std::string name8Bit, ID id)
    : Symbol(name), name8Bit{name8Bit}, id{id} {}
std::string Register::toStr() const { return name; }
void Register::accept(Visitor &visitor) const { visitor.visit(this); }
const std::string Register::getName8Bit() const { return name8Bit; }
Register::ID Register::getID() const { return id; }
const std::unordered_set<const Register *> &Register::getAllGPRegisters() { return allGPRegisters; }
const std::unordered_set<const Register *> &Register::getCallerSavedRegisters() {
  return callerSavedRegisters;
}
const std::unordered_set<const Register *> &Register::getCalleeSavedRegisters() {
  return calleeSavedRegisters;
}
const std::vector<const Register *> &Register::getArgRegisters() { return argRegisters; }
const Register *Register::getRegister(ID id) { return enumMap.at(id); }
const std::unordered_map<Register::ID, const Register *> Register::enumMap = {
    {ID::RAX, new Register("rax", "al", ID::RAX)},
    {ID::RBX, new Register("rbx", "bl", ID::RBX)},
    {ID::RCX, new Register("rcx", "cl", ID::RCX)},
    {ID::RDX, new Register("rdx", "dl", ID::RDX)},
    {ID::RDI, new Register("rdi", "dil", ID::RDI)},
    {ID::RSI, new Register("rsi", "sil", ID::RSI)},
    {ID::RBP, new Register("rbp", "bpl", ID::RBP)},
    {ID::RSP, new Register("rsp", "<illegal>", ID::RSP)},
    {ID::R8, new Register("r8", "r8b", ID::R8)},
    {ID::R9, new Register("r9", "r9b", ID::R9)},
    {ID::R10, new Register("r10", "r10b", ID::R10)},
    {ID::R11, new Register("r11", "r11b", ID::R11)},
    {ID::R12, new Register("r12", "r12b", ID::R12)},
    {ID::R13, new Register("r13", "r13b", ID::R13)},
    {ID::R14, new Register("r14", "r14b", ID::R14)},
    {ID::R15, new Register("r15", "r15b", ID::R15)}};
const std::unordered_set<const Register *> Register::allGPRegisters = {
    enumMap.at(ID::RAX), enumMap.at(ID::RBX), enumMap.at(ID::RCX), enumMap.at(ID::RDX),
    enumMap.at(ID::RDI), enumMap.at(ID::RSI), enumMap.at(ID::RBP), enumMap.at(ID::R8),
    enumMap.at(ID::R9),  enumMap.at(ID::R10), enumMap.at(ID::R11), enumMap.at(ID::R12),
    enumMap.at(ID::R13), enumMap.at(ID::R14), enumMap.at(ID::R15)};
const std::unordered_set<const Register *> Register::callerSavedRegisters = {
    enumMap.at(ID::RAX), enumMap.at(ID::RDI), enumMap.at(ID::RSI),
    enumMap.at(ID::RDX), enumMap.at(ID::RCX), enumMap.at(ID::R8),
    enumMap.at(ID::R9),  enumMap.at(ID::R10), enumMap.at(ID::R11)};
const std::unordered_set<const Register *> Register::calleeSavedRegisters = {
    enumMap.at(ID::RBP), enumMap.at(ID::RBX), enumMap.at(ID::R12),
    enumMap.at(ID::R13), enumMap.at(ID::R14), enumMap.at(ID::R15)};
const std::vector<const Register *> Register::argRegisters = {
    enumMap.at(ID::RDI), enumMap.at(ID::RSI), enumMap.at(ID::RDX),
    enumMap.at(ID::RCX), enumMap.at(ID::R8),  enumMap.at(ID::R9)};

Variable::Variable(std::string name) : Symbol(name) {}
std::string Variable::toStr() const { return name; }
void Variable::accept(Visitor &visitor) const { visitor.visit(this); }

Number::Number(int64_t val) : val{val} {}
int64_t Number::getVal() const { return val; }
std::string Number::toStr() const { return std::to_string(val); }
void Number::accept(Visitor &visitor) const { visitor.visit(this); }

CompareOp::CompareOp(std::string name) : name{name} {}
CompareOp *CompareOp::getCompareOp(ID id) { return enumMap.at(id); }
const std::string CompareOp::getName() const { return name; }
std::string CompareOp::toStr() const { return name; }
void CompareOp::accept(Visitor &visitor) const { visitor.visit(this); }
const std::unordered_map<CompareOp::ID, CompareOp *> CompareOp::enumMap = {
    {ID::LESS_THAN, new CompareOp("<")},
    {ID::LESS_EQUAL, new CompareOp("<=")},
    {ID::EQUAL, new CompareOp("=")}};

ShiftOp::ShiftOp(std::string name) : name{name} {}
ShiftOp *ShiftOp::getShiftOp(ID id) { return enumMap.at(id); }
const std::string ShiftOp::getName() const { return name; }
std::string ShiftOp::toStr() const { return name; }
void ShiftOp::accept(Visitor &visitor) const { visitor.visit(this); }
const std::unordered_map<ShiftOp::ID, ShiftOp *> ShiftOp::enumMap = {
    {ID::LEFT, new ShiftOp("<<=")}, {ID::RIGHT, new ShiftOp(">>=")}};

ArithOp::ArithOp(std::string name) : name{name} {}
ArithOp *ArithOp::getArithOp(ID id) { return enumMap.at(id); }
const std::string ArithOp::getName() const { return name; }
std::string ArithOp::toStr() const { return name; }
void ArithOp::accept(Visitor &visitor) const { visitor.visit(this); }
const std::unordered_map<ArithOp::ID, ArithOp *> ArithOp::enumMap = {{ID::ADD, new ArithOp("+=")},
                                                                     {ID::SUB, new ArithOp("-=")},
                                                                     {ID::MUL, new ArithOp("*=")},
                                                                     {ID::AND, new ArithOp("&=")}};

SelfModOp::SelfModOp(std::string name) : name{name} {}
SelfModOp *SelfModOp::getSelfModOp(ID id) { return enumMap.at(id); }
const std::string SelfModOp::getName() const { return name; }
std::string SelfModOp::toStr() const { return name; }
void SelfModOp::accept(Visitor &visitor) const { visitor.visit(this); }
const std::unordered_map<SelfModOp::ID, SelfModOp *> SelfModOp::enumMap = {
    {ID::INC, new SelfModOp("++")}, {ID::DEC, new SelfModOp("--")}};

MemoryLocation::MemoryLocation(const Symbol *base, const Number *offset)
    : base{base}, offset{offset} {}
const Symbol *MemoryLocation::getBase() const { return base; }
const Number *MemoryLocation::getOffset() const { return offset; }
std::string MemoryLocation::toStr() const { return "mem " + base->toStr() + " " + offset->toStr(); }
void MemoryLocation::accept(Visitor &visitor) const { visitor.visit(this); }

StackLocation::StackLocation(const Number *offset) : offset{offset} {}
const Number *StackLocation::getOffset() const { return offset; }
std::string StackLocation::toStr() const { return "stack-arg " + offset->toStr(); }
void StackLocation::accept(Visitor &visitor) const { visitor.visit(this); }

FunctionName::FunctionName(std::string name) : name{name} {}
std::string FunctionName::getName() const { return name; }
std::string FunctionName::toStr() const { return name; }
void FunctionName::accept(Visitor &visitor) const { visitor.visit(this); }

Label::Label(std::string name) : name{name} {}
std::string Label::getName() const { return name; }
std::string Label::toStr() const { return name; }
void Label::accept(Visitor &visitor) const { visitor.visit(this); }

/*
 *  Instructions.
 */
std::string RetInst::toStr() const { return "return"; }
void RetInst::accept(Visitor &visitor) const { visitor.visit(this); }

ShiftInst::ShiftInst(const ShiftOp *op, const Symbol *lval, const Value *rval)
    : op{op}, lval{lval}, rval{rval} {}
const ShiftOp *ShiftInst::getOp() const { return op; }
const Symbol *ShiftInst::getLval() const { return lval; }
const Value *ShiftInst::getRval() const { return rval; }
std::string ShiftInst::toStr() const {
  return lval->toStr() + " " + op->toStr() + " " + rval->toStr();
}
void ShiftInst::accept(Visitor &visitor) const { visitor.visit(this); }

ArithInst::ArithInst(const ArithOp *op, const Item *lval, const Item *rval)
    : op{op}, lval{lval}, rval{rval} {}
const ArithOp *ArithInst::getOp() const { return op; }
const Item *ArithInst::getLval() const { return lval; }
const Item *ArithInst::getRval() const { return rval; }
std::string ArithInst::toStr() const {
  return lval->toStr() + " " + op->toStr() + " " + rval->toStr();
}
void ArithInst::accept(Visitor &visitor) const { visitor.visit(this); }

SelfModInst::SelfModInst(const SelfModOp *op, const Symbol *lval) : op{op}, lval{lval} {}
const SelfModOp *SelfModInst::getOp() const { return op; }
const Symbol *SelfModInst::getLval() const { return lval; }
std::string SelfModInst::toStr() const { return lval->toStr() + op->toStr(); }
void SelfModInst::accept(Visitor &visitor) const { visitor.visit(this); }

AssignInst::AssignInst(const Item *lval, const Item *rval) : lval{lval}, rval{rval} {}
const Item *AssignInst::getLval() const { return lval; }
const Item *AssignInst::getRval() const { return rval; }
std::string AssignInst::toStr() const { return lval->toStr() + " <- " + rval->toStr(); }
void AssignInst::accept(Visitor &visitor) const { visitor.visit(this); }

CompareAssignInst::CompareAssignInst(const Symbol *lval, const CompareOp *op, const Value *cmpLval,
                                     const Value *cmpRval)
    : lval{lval}, op{op}, cmpLval{cmpLval}, cmpRval{cmpRval} {}
const Symbol *CompareAssignInst::getLval() const { return lval; }
const CompareOp *CompareAssignInst::getOp() const { return op; }
const Value *CompareAssignInst::getCmpLval() const { return cmpLval; }
const Value *CompareAssignInst::getCmpRval() const { return cmpRval; }
std::string CompareAssignInst::toStr() const {
  return lval->toStr() + " <- " + cmpLval->toStr() + " " + op->toStr() + " " + cmpRval->toStr();
}
void CompareAssignInst::accept(Visitor &visitor) const { visitor.visit(this); }

CallInst::CallInst(const Item *callee, const Number *argNum) : callee{callee}, argNum{argNum} {}
const Item *CallInst::getCallee() const { return callee; }
const Number *CallInst::getArgNum() const { return argNum; }
std::string CallInst::toStr() const { return "call " + callee->toStr() + " " + argNum->toStr(); }
void CallInst::accept(Visitor &visitor) const { visitor.visit(this); }

std::string PrintInst::toStr() const { return "call print 1"; }
void PrintInst::accept(Visitor &visitor) const { visitor.visit(this); }

std::string InputInst::toStr() const { return "call input 0"; }
void InputInst::accept(Visitor &visitor) const { visitor.visit(this); }

std::string AllocateInst::toStr() const { return "call allocate 2"; }
void AllocateInst::accept(Visitor &visitor) const { visitor.visit(this); }

std::string TupleErrorInst::toStr() const { return "call tuple-error 0"; }
void TupleErrorInst::accept(Visitor &visitor) const { visitor.visit(this); }

TensorErrorInst::TensorErrorInst(const Number *argNum) : argNum(argNum) {}
const Number *TensorErrorInst::getArgNum() const { return argNum; }
std::string TensorErrorInst::toStr() const { return "call tensor-error " + argNum->toStr(); }
void TensorErrorInst::accept(Visitor &visitor) const { visitor.visit(this); }

SetInst::SetInst(const Symbol *lval, const Symbol *base, const Symbol *offset, const Number *scalar)
    : lval{lval}, base{base}, offset{offset}, scalar{scalar} {}
const Symbol *SetInst::getLval() const { return lval; }
const Symbol *SetInst::getBase() const { return base; }
const Symbol *SetInst::getOffset() const { return offset; }
const Number *SetInst::getScalar() const { return scalar; }
std::string SetInst::toStr() const {
  return lval->toStr() + " @ " + base->toStr() + " " + offset->toStr() + " " + scalar->toStr();
}
void SetInst::accept(Visitor &visitor) const { visitor.visit(this); }

LabelInst::LabelInst(const Label *label) : label{label} {}
const Label *LabelInst::getLabel() const { return label; }
std::string LabelInst::toStr() const { return label->toStr(); }
void LabelInst::accept(Visitor &visitor) const { visitor.visit(this); }

GotoInst::GotoInst(const Label *label) : label{label} {}
const Label *GotoInst::getLabel() const { return label; }
std::string GotoInst::toStr() const { return "goto " + label->toStr(); }
void GotoInst::accept(Visitor &visitor) const { visitor.visit(this); }

CondJumpInst::CondJumpInst(const CompareOp *op, const Value *lval, const Value *rval,
                           const Label *label)
    : op{op}, lval{lval}, rval{rval}, label{label} {}
const Label *CondJumpInst::getLabel() const { return label; }
const Value *CondJumpInst::getLval() const { return lval; }
const Value *CondJumpInst::getRval() const { return rval; }
const CompareOp *CondJumpInst::getOp() const { return op; }
std::string CondJumpInst::toStr() const {
  return "cjump " + lval->toStr() + " " + op->toStr() + " " + rval->toStr() + " " + label->toStr();
}
void CondJumpInst::accept(Visitor &visitor) const { visitor.visit(this); }

const std::vector<const Instruction *> &BasicBlock::getInstructions() const { return instructions; }
void BasicBlock::addInstruction(const Instruction *inst) { instructions.push_back(inst); }
const std::unordered_set<BasicBlock *> &BasicBlock::getSuccessors() const { return successors; }
void BasicBlock::addSuccessor(BasicBlock *BB) { successors.insert(BB); }
void BasicBlock::removeSuccessor(BasicBlock *BB) { successors.erase(BB); }
const std::unordered_set<BasicBlock *> &BasicBlock::getPredecessors() const { return predecessors; }
void BasicBlock::addPredecessor(BasicBlock *BB) { predecessors.insert(BB); }
void BasicBlock::removePredecessor(BasicBlock *BB) { predecessors.erase(BB); }
const Instruction *BasicBlock::getFirstInstruction() const { return instructions.front(); }
const Instruction *BasicBlock::getTerminator() const { return instructions.back(); }

Function::Function(std::string name) : name{name} {
  // start with an empty basic block
  basicBlocks.push_back(new BasicBlock());
}
std::string Function::getName() const { return name; }
int64_t Function::getParamNum() const  { return paramNum; }
void Function::setParameters(int64_t parameters) { this->paramNum = parameters; }
const std::vector<BasicBlock *> &Function::getBasicBlocks() const { return basicBlocks; }
void Function::addBasicBlock(BasicBlock *BB) { basicBlocks.push_back(BB); }
BasicBlock *Function::getCurrBasicBlock() const { return basicBlocks.back(); }
void Function::popCurrBasicBlock() { basicBlocks.pop_back(); }
const Variable *Function::getVariable(std::string name) {
  if (variables.find(name) == variables.end())
    variables[name] = new Variable(name);
  return variables[name];
}
bool Function::hasVariable(std::string name) const {
  return variables.find(name) != variables.end();
}
const std::unordered_map<std::string, const Variable *> &Function::getVariables() const {
  return variables;
}

std::string Program::getEntryPointLabel() const { return entryPointLabel; }
void Program::setEntryPointLabel(std::string label) { entryPointLabel = label; }
const std::vector<Function *> &Program::getFunctions() const { return functions; }
void Program::addFunction(Function *F) { functions.push_back(F); }
Function *Program::getCurrFunction() const { return functions.back(); }

} // namespace L2
