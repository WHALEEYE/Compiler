#include <L2.h>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace L2 {

Symbol::Symbol(std::string name) : name{name} {}
std::string Symbol::getName() { return name; }

Register::Register(std::string name, std::string name8Bit) : Symbol(name), name8Bit{name8Bit} {}
std::string Register::toStr() { return name; }
void Register::accept(Visitor &visitor) { visitor.visit(this); }
std::string Register::getName8Bit() { return name8Bit; }
const std::unordered_set<Register *> &Register::getAllRegisters() { return allRegisters; }
const std::unordered_set<Register *> &Register::getCallerSavedRegisters() {
  return callerSavedRegisters;
}
const std::unordered_set<Register *> &Register::getCalleeSavedRegisters() {
  return calleeSavedRegisters;
}
const std::vector<Register *> &Register::getArgRegisters() { return argRegisters; }
Register *Register::getRegister(ID id) { return enumMap.at(id); }
const std::unordered_map<Register::ID, Register *> Register::enumMap = {
    {ID::R8, new Register("r8", "r8b")},    {ID::R9, new Register("r9", "r9b")},
    {ID::R10, new Register("r10", "r10b")}, {ID::R11, new Register("r11", "r11b")},
    {ID::R12, new Register("r12", "r12b")}, {ID::R13, new Register("r13", "r13b")},
    {ID::R14, new Register("r14", "r14b")}, {ID::R15, new Register("r15", "r15b")},
    {ID::RAX, new Register("rax", "al")},   {ID::RBX, new Register("rbx", "bl")},
    {ID::RCX, new Register("rcx", "cl")},   {ID::RDX, new Register("rdx", "dl")},
    {ID::RDI, new Register("rdi", "dil")},  {ID::RSI, new Register("rsi", "sil")},
    {ID::RBP, new Register("rbp", "bpl")},  {ID::RSP, new Register("rsp", "<unknown-token>")},
};
const std::unordered_set<Register *> Register::allRegisters = {
    enumMap.at(ID::RAX), enumMap.at(ID::RBX), enumMap.at(ID::RCX), enumMap.at(ID::RDX),
    enumMap.at(ID::RDI), enumMap.at(ID::RSI), enumMap.at(ID::RBP), enumMap.at(ID::RSP),
    enumMap.at(ID::R8),  enumMap.at(ID::R9),  enumMap.at(ID::R10), enumMap.at(ID::R11),
    enumMap.at(ID::R12), enumMap.at(ID::R13), enumMap.at(ID::R14), enumMap.at(ID::R15)};
const std::unordered_set<Register *> Register::callerSavedRegisters = {
    enumMap.at(ID::RAX), enumMap.at(ID::RDI), enumMap.at(ID::RSI),
    enumMap.at(ID::RDX), enumMap.at(ID::RCX), enumMap.at(ID::R8),
    enumMap.at(ID::R9),  enumMap.at(ID::R10), enumMap.at(ID::R11)};
const std::unordered_set<Register *> Register::calleeSavedRegisters = {
    enumMap.at(ID::RBP), enumMap.at(ID::RBX), enumMap.at(ID::R12),
    enumMap.at(ID::R13), enumMap.at(ID::R14), enumMap.at(ID::R15)};
const std::vector<Register *> Register::argRegisters = {enumMap.at(ID::RDI), enumMap.at(ID::RSI),
                                                        enumMap.at(ID::RDX), enumMap.at(ID::RCX),
                                                        enumMap.at(ID::R8),  enumMap.at(ID::R9)};

Variable::Variable(std::string name) : Symbol(name) {}
std::string Variable::toStr() { return name; }
void Variable::accept(Visitor &visitor) { visitor.visit(this); }

Number::Number(int64_t val) : val{val} {}
int64_t Number::getVal() { return val; }
std::string Number::toStr() { return std::to_string(val); }
void Number::accept(Visitor &visitor) { visitor.visit(this); }

CompareOp::CompareOp(std::string name) : name{name} {}
CompareOp *CompareOp::getCompareOp(ID id) { return enumMap.at(id); }
std::string CompareOp::getName() { return name; }
std::string CompareOp::toStr() { return name; }
void CompareOp::accept(Visitor &visitor) { visitor.visit(this); }
const std::unordered_map<CompareOp::ID, CompareOp *> CompareOp::enumMap = {
    {ID::LESS_THAN, new CompareOp("<")},
    {ID::LESS_EQUAL, new CompareOp("<=")},
    {ID::EQUAL, new CompareOp("=")}};

ShiftOp::ShiftOp(std::string name) : name{name} {}
ShiftOp *ShiftOp::getShiftOp(ID id) { return enumMap.at(id); }
std::string ShiftOp::getName() { return name; }
std::string ShiftOp::toStr() { return name; }
void ShiftOp::accept(Visitor &visitor) { visitor.visit(this); }
const std::unordered_map<ShiftOp::ID, ShiftOp *> ShiftOp::enumMap = {
    {ID::LEFT, new ShiftOp("<<=")}, {ID::RIGHT, new ShiftOp(">>=")}};

ArithOp::ArithOp(std::string name) : name{name} {}
ArithOp *ArithOp::getArithOp(ID id) { return enumMap.at(id); }
std::string ArithOp::getName() { return name; }
std::string ArithOp::toStr() { return name; }
void ArithOp::accept(Visitor &visitor) { visitor.visit(this); }
const std::unordered_map<ArithOp::ID, ArithOp *> ArithOp::enumMap = {{ID::ADD, new ArithOp("+=")},
                                                                     {ID::SUB, new ArithOp("-=")},
                                                                     {ID::MUL, new ArithOp("*=")},
                                                                     {ID::AND, new ArithOp("&=")}};

SelfModOp::SelfModOp(std::string name) : name{name} {}
SelfModOp *SelfModOp::getSelfModOp(ID id) { return enumMap.at(id); }
std::string SelfModOp::getName() { return name; }
std::string SelfModOp::toStr() { return name; }
void SelfModOp::accept(Visitor &visitor) { visitor.visit(this); }
const std::unordered_map<SelfModOp::ID, SelfModOp *> SelfModOp::enumMap = {
    {ID::INC, new SelfModOp("++")}, {ID::DEC, new SelfModOp("--")}};

MemoryLocation::MemoryLocation(Item *base, Number *offset) : base{base}, offset{offset} {}
Item *MemoryLocation::getBase() { return base; }
Number *MemoryLocation::getOffset() { return offset; }
std::string MemoryLocation::toStr() { return "mem " + base->toStr() + " " + offset->toStr(); }
void MemoryLocation::accept(Visitor &visitor) { visitor.visit(this); }

StackLocation::StackLocation(Number *offset) : offset{offset} {}
Number *StackLocation::getOffset() { return offset; }
std::string StackLocation::toStr() { return "stack-arg " + offset->toStr(); }
void StackLocation::accept(Visitor &visitor) { visitor.visit(this); }

FunctionName::FunctionName(std::string name) : name{name} {}
std::string FunctionName::getName() { return name; }
std::string FunctionName::toStr() { return name; }
void FunctionName::accept(Visitor &visitor) { visitor.visit(this); }

Label::Label(std::string name) : name{name} {}
std::string Label::getName() { return name; }
std::string Label::toStr() { return name; }
void Label::accept(Visitor &visitor) { visitor.visit(this); }

/*
 *  Instructions.
 */
std::string RetInst::toStr() { return "return"; }
void RetInst::accept(Visitor &visitor) { visitor.visit(this); }

ShiftInst::ShiftInst(ShiftOp *op, Symbol *lval, Value *rval) : op{op}, lval{lval}, rval{rval} {}
ShiftOp *ShiftInst::getOp() { return op; }
Symbol *ShiftInst::getLval() { return lval; }
Value *ShiftInst::getRval() { return rval; }
std::string ShiftInst::toStr() { return lval->toStr() + " " + op->toStr() + " " + rval->toStr(); }
void ShiftInst::accept(Visitor &visitor) { visitor.visit(this); }

ArithInst::ArithInst(ArithOp *op, Item *lval, Item *rval) : op{op}, lval{lval}, rval{rval} {}
ArithOp *ArithInst::getOp() { return op; }
Item *ArithInst::getLval() { return lval; }
Item *ArithInst::getRval() { return rval; }
std::string ArithInst::toStr() { return lval->toStr() + " " + op->toStr() + " " + rval->toStr(); }
void ArithInst::accept(Visitor &visitor) { visitor.visit(this); }

SelfModInst::SelfModInst(SelfModOp *op, Symbol *lval) : op{op}, lval{lval} {}
SelfModOp *SelfModInst::getOp() { return op; }
Symbol *SelfModInst::getLval() { return lval; }
std::string SelfModInst::toStr() { return lval->toStr() + " " + op->toStr(); }
void SelfModInst::accept(Visitor &visitor) { visitor.visit(this); }

AssignInst::AssignInst(Item *lval, Item *rval) : lval{lval}, rval{rval} {}
Item *AssignInst::getLval() { return lval; }
Item *AssignInst::getRval() { return rval; }
std::string AssignInst::toStr() { return lval->toStr() + " <- " + rval->toStr(); }
void AssignInst::accept(Visitor &visitor) { visitor.visit(this); }

CompareAssignInst::CompareAssignInst(Symbol *lval, CompareOp *op, Value *cmpLval, Value *cmpRval)
    : lval{lval}, op{op}, cmpLval{cmpLval}, cmpRval{cmpRval} {}
Symbol *CompareAssignInst::getLval() { return lval; }
CompareOp *CompareAssignInst::getOp() { return op; }
Value *CompareAssignInst::getCmpLval() { return cmpLval; }
Value *CompareAssignInst::getCmpRval() { return cmpRval; }
std::string CompareAssignInst::toStr() {
  return lval->toStr() + " <- " + cmpLval->toStr() + " " + op->toStr() + " " + cmpRval->toStr();
}
void CompareAssignInst::accept(Visitor &visitor) { visitor.visit(this); }

CallInst::CallInst(Item *callee, Number *argNum) : callee{callee}, argNum{argNum} {}
Item *CallInst::getCallee() { return callee; }
Number *CallInst::getArgNum() { return argNum; }
std::string CallInst::toStr() { return "call " + callee->toStr() + " " + argNum->toStr(); }
void CallInst::accept(Visitor &visitor) { visitor.visit(this); }

std::string PrintInst::toStr() { return "call print 1"; }
void PrintInst::accept(Visitor &visitor) { visitor.visit(this); }

std::string InputInst::toStr() { return "call input 0"; }
void InputInst::accept(Visitor &visitor) { visitor.visit(this); }

std::string AllocateInst::toStr() { return "call allocate 2"; }
void AllocateInst::accept(Visitor &visitor) { visitor.visit(this); }

std::string TupleErrorInst::toStr() { return "call tuple-error 0"; }
void TupleErrorInst::accept(Visitor &visitor) { visitor.visit(this); }

TensorErrorInst::TensorErrorInst(Number *argNum) : argNum(argNum) {}
Number *TensorErrorInst::getArgNum() { return argNum; }
std::string TensorErrorInst::toStr() { return "call tensor-error " + argNum->toStr(); }
void TensorErrorInst::accept(Visitor &visitor) { visitor.visit(this); }

SetInst::SetInst(Symbol *lval, Symbol *base, Symbol *offset, Number *scalar)
    : lval{lval}, base{base}, offset{offset}, scalar{scalar} {}
Symbol *SetInst::getLval() { return lval; }
Symbol *SetInst::getBase() { return base; }
Symbol *SetInst::getOffset() { return offset; }
Number *SetInst::getScalar() { return scalar; }
std::string SetInst::toStr() {
  return lval->toStr() + " @ " + base->toStr() + " " + offset->toStr() + " " + scalar->toStr();
}
void SetInst::accept(Visitor &visitor) { visitor.visit(this); }

LabelInst::LabelInst(Label *label) : label{label} {}
Label *LabelInst::getLabel() { return label; }
std::string LabelInst::toStr() { return label->toStr(); }
void LabelInst::accept(Visitor &visitor) { visitor.visit(this); }

GotoInst::GotoInst(Label *label) : label{label} {}
Label *GotoInst::getLabel() { return label; }
std::string GotoInst::toStr() { return "goto " + label->toStr(); }
void GotoInst::accept(Visitor &visitor) { visitor.visit(this); }

CondJumpInst::CondJumpInst(CompareOp *op, Value *lval, Value *rval, Label *label)
    : op{op}, lval{lval}, rval{rval}, label{label} {}
Label *CondJumpInst::getLabel() { return label; }
Value *CondJumpInst::getLval() { return lval; }
Value *CondJumpInst::getRval() { return rval; }
CompareOp *CondJumpInst::getOp() { return op; }
std::string CondJumpInst::toStr() {
  return "cjump " + lval->toStr() + " " + op->toStr() + " " + rval->toStr() + " " + label->toStr();
}
void CondJumpInst::accept(Visitor &visitor) { visitor.visit(this); }

const std::vector<Instruction *> &BasicBlock::getInstructions() { return instructions; }
void BasicBlock::addInstruction(Instruction *inst) { instructions.push_back(inst); }
const std::unordered_set<BasicBlock *> &BasicBlock::getSuccessors() { return successors; }
void BasicBlock::addSuccessor(BasicBlock *BB) { successors.insert(BB); }
void BasicBlock::removeSuccessor(BasicBlock *BB) { successors.erase(BB); }
const std::unordered_set<BasicBlock *> &BasicBlock::getPredecessors() { return predecessors; }
void BasicBlock::addPredecessor(BasicBlock *BB) { predecessors.insert(BB); }
void BasicBlock::removePredecessor(BasicBlock *BB) { predecessors.erase(BB); }
Instruction *BasicBlock::getFirstInstruction() { return instructions.front(); }
Instruction *BasicBlock::getTerminator() { return instructions.back(); }

Function::Function(std::string name) : name{name} {
  // start with an empty basic block
  basicBlocks.push_back(new BasicBlock());
}
std::string Function::getName() { return name; }
int64_t Function::getParamNum() { return paramNum; }
void Function::setParameters(int64_t parameters) { this->paramNum = parameters; }
const std::vector<BasicBlock *> &Function::getBasicBlocks() { return basicBlocks; }
void Function::addBasicBlock(BasicBlock *BB) { basicBlocks.push_back(BB); }
BasicBlock *Function::getCurrBasicBlock() { return basicBlocks.back(); }
void Function::popCurrBasicBlock() { basicBlocks.pop_back(); }
Variable *Function::getVariable(std::string name) {
  if (variables.find(name) == variables.end())
    variables[name] = new Variable(name);
  return variables[name];
}

std::string Program::getEntryPointLabel() { return entryPointLabel; }
void Program::setEntryPointLabel(std::string label) { entryPointLabel = label; }
const std::vector<Function *> &Program::getFunctions() { return functions; }
void Program::addFunction(Function *F) { functions.push_back(F); }
Function *Program::getCurrFunction() { return functions.back(); }

} // namespace L2
