#include <L2.h>
#include <cstdint>
#include <string>

namespace L2 {

const std::string regName[] = {"r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
                               "rax", "rbx", "rcx", "rdx", "rdi", "rsi", "rbp", "rsp"};
const std::string reg8BitName[] = {"r8b",  "r9b",  "r10b", "r11b",         "r12b", "r13b",
                                   "r14b", "r15b", "al",   "bl",           "cl",   "dl",
                                   "dil",  "sil",  "bpl",  "<unknown-reg>"};

const std::string cmpOpName[] = {"<", "<=", "="};

const std::string shiftOpName[] = {"<<=", ">>="};

const std::string arithOpName[] = {"+=", "-=", "*=", "&="};

const std::string selfModOpName[] = {"++", "--"};

Register::Register(RegisterID id) : id{id} { return; }
RegisterID Register::getID() { return id; }
std::string Register::toStr() { return getName(); }
std::string Register::getName() { return regName[id]; }
std::string Register::get8BitName() { return reg8BitName[id]; }

Variable::Variable(std::string name) : name{name.substr(1)} { return; }
std::string Variable::getName() { return name; }
std::string Variable::toStr() { return "%" + name; }

Number::Number(int64_t val) : val{val} { return; }
int64_t Number::getVal() { return val; }
std::string Number::toStr() { return std::to_string(val); }

CompareOp::CompareOp(CompareOpID id) : id{id} { return; }
CompareOpID CompareOp::getID() { return id; }
std::string CompareOp::toStr() { return cmpOpName[id]; }

ShiftOp::ShiftOp(ShiftOpID id) : id{id} { return; }
ShiftOpID ShiftOp::getID() { return id; }
std::string ShiftOp::toStr() { return shiftOpName[id]; }

ArithOp::ArithOp(ArithOpID id) : id{id} { return; }
ArithOpID ArithOp::getID() { return id; }
std::string ArithOp::toStr() { return arithOpName[id]; }

SelfModOp::SelfModOp(SelfModOpID id) : id{id} { return; }
SelfModOpID SelfModOp::getID() { return id; }
std::string SelfModOp::toStr() { return selfModOpName[id]; }

MemoryLocation::MemoryLocation(Register *reg, Number *offset) : reg{reg}, offset{offset} { return; }
Register *MemoryLocation::getReg() { return reg; }
Number *MemoryLocation::getOffset() { return offset; }
std::string MemoryLocation::toStr() { return "mem " + reg->toStr() + " " + offset->toStr(); }

StackLocation::StackLocation(Number *offset) : offset{offset} { return; }
Number *StackLocation::getOffset() { return offset; }
std::string StackLocation::toStr() { return "stack-arg " + offset->toStr(); }

FunctionName::FunctionName(std::string name) : name{name.substr(1)} { return; }
std::string FunctionName::getName() { return name; }
std::string FunctionName::toStr() { return '@' + name; }

Label::Label(std::string name) : name{name.substr(1)} { return; }
std::string Label::getName() { return name; }
std::string Label::toStr() { return ':' + name; }

/*
 *  Instructions.
 */
std::string RetInst::toStr() { return "return"; }

ShiftInst::ShiftInst(ShiftOp *op, Item *lval, Item *rval) : op{op}, lval{lval}, rval{rval} {
  return;
}
ShiftOp *ShiftInst::getOp() { return op; }
Item *ShiftInst::getLval() { return lval; }
Item *ShiftInst::getRval() { return rval; }
std::string ShiftInst::toStr() { return lval->toStr() + " " + op->toStr() + " " + rval->toStr(); }

ArithInst::ArithInst(ArithOp *op, Item *lval, Item *rval) : op{op}, lval{lval}, rval{rval} {
  return;
}
ArithOp *ArithInst::getOp() { return op; }
Item *ArithInst::getLval() { return lval; }
Item *ArithInst::getRval() { return rval; }
std::string ArithInst::toStr() { return lval->toStr() + " " + op->toStr() + " " + rval->toStr(); }

SelfModInst::SelfModInst(SelfModOp *op, Item *lval) : op{op}, lval{lval} { return; }
SelfModOp *SelfModInst::getOp() { return op; }
Item *SelfModInst::getLval() { return lval; }
std::string SelfModInst::toStr() { return lval->toStr() + " " + op->toStr(); }

AssignInst::AssignInst(Item *lval, Item *rval) : lval{lval}, rval{rval} { return; }
Item *AssignInst::getLval() { return lval; }
Item *AssignInst::getRval() { return rval; }
std::string AssignInst::toStr() { return lval->toStr() + " <- " + rval->toStr(); }

CompareAssignInst::CompareAssignInst(Register *lval, CompareOp *op, Item *cmpLval, Item *cmpRval)
    : lval{lval}, op{op}, cmpLval{cmpLval}, cmpRval{cmpRval} {
  return;
}
Register *CompareAssignInst::getLval() { return lval; }
CompareOp *CompareAssignInst::getOp() { return op; }
Item *CompareAssignInst::getCmpLval() { return cmpLval; }
Item *CompareAssignInst::getCmpRval() { return cmpRval; }
std::string CompareAssignInst::toStr() {
  return lval->toStr() + " <- " + cmpLval->toStr() + " " + op->toStr() + " " + cmpRval->toStr();
}

CallInst::CallInst(Item *callee, Number *argNum) : callee{callee}, argNum{argNum} { return; }
Item *CallInst::getCallee() { return callee; }
Number *CallInst::getArgNum() { return argNum; }
std::string CallInst::toStr() { return "call " + callee->toStr() + " " + argNum->toStr(); }

std::string PrintInst::toStr() { return "call print 1"; }

std::string InputInst::toStr() { return "call input 0"; }

std::string AllocateInst::toStr() { return "call allocate 2"; }

std::string TupleErrorInst::toStr() { return "call tuple-error 0"; }

TensorErrorInst::TensorErrorInst(Number *number) : number(number) { return; }
Number *TensorErrorInst::getNumber() { return number; }
std::string TensorErrorInst::toStr() { return "call tensor-error " + number->toStr(); }

SetInst::SetInst(Register *lval, Register *base, Register *offset, Number *scalar)
    : lval{lval}, base{base}, offset{offset}, scalar{scalar} {
  return;
}
Register *SetInst::getLval() { return lval; }
Register *SetInst::getBase() { return base; }
Register *SetInst::getOffset() { return offset; }
Number *SetInst::getScalar() { return scalar; }
std::string SetInst::toStr() {
  return lval->toStr() + " @ " + base->toStr() + " " + offset->toStr() + " " + scalar->toStr();
}

LabelInst::LabelInst(Label *label) : label{label} { return; }
Label *LabelInst::getLabel() { return label; }
std::string LabelInst::toStr() { return label->toStr(); }

GotoInst::GotoInst(Label *label) : label{label} { return; }
Label *GotoInst::getLabel() { return label; }
std::string GotoInst::toStr() { return "goto " + label->toStr(); }

CondJumpInst::CondJumpInst(CompareOp *op, Item *lval, Item *rval, Label *label)
    : op{op}, lval{lval}, rval{rval}, label{label} {
  return;
}
Label *CondJumpInst::getLabel() { return label; }
Item *CondJumpInst::getLval() { return lval; }
Item *CondJumpInst::getRval() { return rval; }
CompareOp *CondJumpInst::getOp() { return op; }
std::string CondJumpInst::toStr() {
  return "cjump " + lval->toStr() + " " + op->toStr() + " " + rval->toStr() + " " + label->toStr();
}

const std::vector<Instruction *> &BasicBlock::getInstructions() { return instructions; }
void BasicBlock::addInstruction(Instruction *inst) { instructions.push_back(inst); }
const std::vector<BasicBlock *> &BasicBlock::getSuccessors() { return successors; }
void BasicBlock::addSuccessor(BasicBlock *BB) { successors.push_back(BB); }
const std::vector<BasicBlock *> &BasicBlock::getPredecessors() { return predecessors; }
void BasicBlock::addPredecessor(BasicBlock *BB) { predecessors.push_back(BB); }
Instruction *BasicBlock::getFirstInstruction() { return instructions.front(); }
Instruction *BasicBlock::getTerminator() { return instructions.back(); }

Function::Function(std::string name) : name{name.substr(1)} {
  // start with an empty basic block
  basicBlocks.push_back(new BasicBlock());
  return;
}
std::string Function::getName() { return name; }
int64_t Function::getParamNum() { return paramNum; }
void Function::setParameters(int64_t parameters) { this->paramNum = parameters; }
const std::vector<BasicBlock *> &Function::getBasicBlocks() { return basicBlocks; }
void Function::addBasicBlock(BasicBlock *BB) { basicBlocks.push_back(BB); }
BasicBlock *Function::getCurrBasicBlock() { return basicBlocks.back(); }

std::string Program::getEntryPointLabel() { return entryPointLabel; }
void Program::setEntryPointLabel(std::string label) { entryPointLabel = label.substr(1); }
const std::vector<Function *> &Program::getFunctions() { return functions; }
void Program::addFunction(Function *F) { functions.push_back(F); }
Function *Program::getCurrFunction() { return functions.back(); }

} // namespace L2
