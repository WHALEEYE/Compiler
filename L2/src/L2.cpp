#include <L2.h>
#include <string>

namespace L2 {

const std::string regToken[] = {"r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
                                "rax", "rbx", "rcx", "rdx", "rdi", "rsi", "rbp", "rsp"};
const std::string regToken8[] = {"r8b",  "r9b",  "r10b", "r11b",         "r12b", "r13b",
                                 "r14b", "r15b", "al",   "bl",           "cl",   "dl",
                                 "dil",  "sil",  "bpl",  "<unknown-reg>"};

const std::string cmpOpL1Token[] = {"<", "<=", "="};
const std::string cmpOpX86Token[] = {"l", "le", "e"};

const std::string shiftOpL1Token[] = {"<<=", ">>="};
const std::string shiftOpX86Token[] = {"salq", "sarq"};

const std::string arithOpL1Token[] = {"+=", "-=", "*=", "&="};
const std::string arithOpX86Token[] = {"addq", "subq", "imulq", "andq"};

const std::string selfModOpL1Token[] = {"++", "--"};
const std::string selfModOpX86Token[] = {"inc", "dec"};

std::string Item::getL2Token() { return "<unknown-L1-token>"; }
std::string Item::getL1Token() { return "<unknown-x86-token>"; }

Register::Register(RegisterID id) : id{id} { return; }
std::string Register::getL2Token() { return regToken[id]; }
std::string Register::getL1Token() { return "%" + regToken[id]; }
std::string Register::getX86Token8() { return "%" + regToken8[id]; }

Variable::Variable(std::string name) : pureName{name.substr(1)} { return; }
std::string Variable::getPureName() { return pureName; }
std::string Variable::getL2Token() { return "%" + pureName; }
std::string Variable::getL1Token() { return "<unimplemented-token>"; }

Number::Number(int64_t val) : val{val} { return; }
int64_t Number::getVal() { return val; }
std::string Number::getL2Token() { return std::to_string(val); }
std::string Number::getL1Token() { return std::to_string(val); }

CompareOp::CompareOp(CompareOpID id) : id{id} { return; }
CompareOpID CompareOp::getID() { return id; }
std::string CompareOp::getL2Token() { return cmpOpL1Token[id]; }
std::string CompareOp::getL1Token() { return cmpOpX86Token[id]; }

ShiftOp::ShiftOp(ShiftOpID id) : id{id} { return; }
ShiftOpID ShiftOp::getID() { return id; }
std::string ShiftOp::getL2Token() { return shiftOpL1Token[id]; }
std::string ShiftOp::getL1Token() { return shiftOpX86Token[id]; }

ArithOp::ArithOp(ArithOpID id) : id{id} { return; }
ArithOpID ArithOp::getID() { return id; }
std::string ArithOp::getL2Token() { return arithOpL1Token[id]; }
std::string ArithOp::getL1Token() { return arithOpX86Token[id]; }

SelfModOp::SelfModOp(SelfModOpID id) : id{id} { return; }
SelfModOpID SelfModOp::getID() { return id; }
std::string SelfModOp::getL2Token() { return selfModOpL1Token[id]; }
std::string SelfModOp::getL1Token() { return selfModOpX86Token[id]; }

MemoryLocation::MemoryLocation(Register *reg, Number *offset) : reg{reg}, offset{offset} { return; }
Register *MemoryLocation::getReg() { return reg; }
Number *MemoryLocation::getOffset() { return offset; }
std::string MemoryLocation::getL2Token() {
  return "mem " + reg->getL2Token() + " " + offset->getL2Token();
}
std::string MemoryLocation::getL1Token() {
  return offset->getL1Token() + "(" + reg->getL1Token() + ")";
}

StackLocation::StackLocation(Number *offset) : offset{offset} { return; }
Number *StackLocation::getOffset() { return offset; }
std::string StackLocation::getL2Token() { return "stack-arg " + offset->getL2Token(); }
std::string StackLocation::getL1Token() { return offset->getL1Token() + "(%rsp)"; }

FunctionName::FunctionName(std::string name) : pureName{name.substr(1)} { return; }
std::string FunctionName::getPureName() { return pureName; }
std::string FunctionName::getL2Token() { return '@' + pureName; }
std::string FunctionName::getL1Token() { return '_' + pureName; }

Label::Label(std::string name) : pureName{name.substr(1)} { return; }
std::string Label::getPureName() { return pureName; }
std::string Label::getL2Token() { return ':' + pureName; }
std::string Label::getL1Token() { return '_' + pureName; }

/*
 *  Instructions.
 */
std::string Instruction::getL2Inst() { return "<unknown-L1-inst>"; }
std::string Instruction::getL1Inst() { return "<unknown-x86-inst>"; }

std::string RetInst::getL2Inst() { return "return"; }
std::string RetInst::getL1Inst() { return "retq"; }

ShiftInst::ShiftInst(ShiftOp *op, Item *lval, Item *rval) : op{op}, lval{lval}, rval{rval} {
  return;
}
std::string ShiftInst::getL2Inst() {
  return lval->getL2Token() + " " + op->getL2Token() + " " + rval->getL2Token();
}
std::string ShiftInst::getL1Inst() {
  std::string amount = "$" + rval->getL1Token();
  if (dynamic_cast<Register *>(rval)) {
    amount = "%cl";
  }
  return op->getL1Token() + " " + amount + ", " + lval->getL1Token();
}

ArithInst::ArithInst(ArithOp *op, Item *lval, Item *rval) : op{op}, lval{lval}, rval{rval} {
  return;
}
std::string ArithInst::getL2Inst() {
  return lval->getL2Token() + " " + op->getL2Token() + " " + rval->getL2Token();
}
std::string ArithInst::getL1Inst() {
  auto r = rval->getL1Token();
  if (dynamic_cast<Number *>(rval))
    r = "$" + r;
  return op->getL1Token() + " " + r + ", " + lval->getL1Token();
}

SelfModInst::SelfModInst(SelfModOp *op, Item *lval) : op{op}, lval{lval} { return; }
std::string SelfModInst::getL2Inst() { return lval->getL2Token() + " " + op->getL2Token(); }
std::string SelfModInst::getL1Inst() { return op->getL1Token() + " " + lval->getL1Token(); }

AssignInst::AssignInst(Item *lval, Item *rval) : lval{lval}, rval{rval} { return; }
std::string AssignInst::getL2Inst() { return lval->getL2Token() + " <- " + rval->getL2Token(); }
std::string AssignInst::getL1Inst() {
  auto r = rval->getL1Token();
  if (dynamic_cast<FunctionName *>(rval) || dynamic_cast<Number *>(rval) ||
      dynamic_cast<Label *>(rval)) {
    r = '$' + rval->getL1Token();
  }
  return "movq " + r + ", " + lval->getL1Token();
}

CompareAssignInst::CompareAssignInst(Register *lval, CompareOp *op, Item *cmpLval, Item *cmpRval)
    : lval{lval}, op{op}, cmpLval{cmpLval}, cmpRval{cmpRval} {
  return;
}
std::string CompareAssignInst::getL2Inst() {
  return lval->getL2Token() + " <- " + cmpLval->getL2Token() + " " + op->getL2Token() + " " +
         cmpRval->getL2Token();
}
std::string CompareAssignInst::getL1Inst() {
  auto newCmpL = cmpLval->getL1Token();
  auto newCmpR = cmpRval->getL1Token();
  auto reversed = true;
  if (dynamic_cast<Number *>(cmpLval) && dynamic_cast<Number *>(cmpRval)) {
    auto cmpL = dynamic_cast<Number *>(cmpLval);
    auto cmpR = dynamic_cast<Number *>(cmpRval);
    // directly compare
    if (op->getID() == CompareOpID::EQUAL && cmpL->getVal() == cmpR->getVal()) {
      return "movq $1, " + lval->getL1Token();
    } else if (op->getID() == CompareOpID::LESS_EQUAL && cmpL->getVal() <= cmpR->getVal()) {
      return "movq $1, " + lval->getL1Token();
    } else if (op->getID() == CompareOpID::LESS_THAN && cmpL->getVal() < cmpR->getVal()) {
      return "movq $1, " + lval->getL1Token();
    } else {
      return "movq $0, " + lval->getL1Token();
    }
  } else if (dynamic_cast<Number *>(cmpRval)) {
    newCmpL = "$" + cmpRval->getL1Token();
    newCmpR = cmpLval->getL1Token();
    reversed = false;
  } else if (dynamic_cast<Number *>(cmpLval)) {
    newCmpL = '$' + cmpLval->getL1Token();
    newCmpR = cmpRval->getL1Token();
  }
  auto cmpInst = "cmpq " + newCmpL + ", " + newCmpR + "\n  ";

  auto newOp = op->getL1Token();
  if (reversed) {
    if (op->getID() == CompareOpID::LESS_EQUAL) {
      newOp = "ge";
    } else if (op->getID() == CompareOpID::LESS_THAN) {
      newOp = "g";
    }
  }
  auto setInst = "set" + newOp + " " + lval->getX86Token8() + "\n  ";
  auto movInst = "movzbq " + lval->getX86Token8() + ", " + lval->getL1Token();
  return cmpInst + setInst + movInst;
}

CallInst::CallInst(Item *callee, Number *arg_num) : callee{callee}, arg_num{arg_num} { return; }
std::string CallInst::getL2Inst() {
  return "call " + callee->getL2Token() + " " + arg_num->getL2Token();
}
std::string CallInst::getL1Inst() {
  auto jmpInst = "jmp *" + callee->getL1Token();
  if (dynamic_cast<FunctionName *>(callee)) {
    jmpInst = "jmp " + callee->getL1Token();
  }
  auto movAmount = (arg_num->getVal() > 6 ? 8 * (arg_num->getVal() - 6) : 0) + 8;
  auto movRspInst = "subq $" + std::to_string(movAmount) + ", %rsp\n  ";
  return movRspInst + jmpInst;
}

std::string PrintInst::getL2Inst() { return "call print 1"; }
std::string PrintInst::getL1Inst() { return "call print"; }

std::string InputInst::getL2Inst() { return "call input 0"; }
std::string InputInst::getL1Inst() { return "call input"; }

std::string AllocateInst::getL2Inst() { return "call allocate 2"; }
std::string AllocateInst::getL1Inst() { return "call allocate"; }

std::string TupleErrorInst::getL2Inst() { return "call tuple-error 0"; }
std::string TupleErrorInst::getL1Inst() { return "call tuple_error"; }

TensorErrorInst::TensorErrorInst(Number *number) : number(number) { return; }
std::string TensorErrorInst::getL2Inst() { return "call tensor-error " + number->getL2Token(); }
std::string TensorErrorInst::getL1Inst() {
  switch (number->getVal()) {
  case 1:
    return "call array_tensor_error_null";
  case 3:
    return "call array_error";
  case 4:
    return "call tensor_error";
  default:
    return "<unknow-inst>";
  }
}

SetInst::SetInst(Register *lval, Register *base, Register *offset, Number *scalar)
    : lval{lval}, base{base}, offset{offset}, scalar{scalar} {
  return;
}
std::string SetInst::getL2Inst() {
  return lval->getL2Token() + " @ " + base->getL2Token() + " " + offset->getL2Token() + " " +
         scalar->getL2Token();
}
std::string SetInst::getL1Inst() {
  return "lea (" + base->getL1Token() + ", " + offset->getL1Token() + ", " +
         scalar->getL1Token() + "), " + lval->getL1Token();
}

LabelInst::LabelInst(Label *label) : label{label} { return; }
std::string LabelInst::getL2Inst() { return label->getL2Token(); }
std::string LabelInst::getL1Inst() { return label->getL1Token() + ":"; }

GotoInst::GotoInst(Label *label) : label{label} { return; }
std::string GotoInst::getL2Inst() { return "goto " + label->getL2Token(); }
std::string GotoInst::getL1Inst() { return "jmp " + label->getL1Token(); }

CondJumpInst::CondJumpInst(CompareOp *op, Item *lval, Item *rval, Label *label)
    : op{op}, lval{lval}, rval{rval}, label{label} {
  return;
}
std::string CondJumpInst::getL2Inst() {
  return "cjump " + lval->getL2Token() + " " + op->getL2Token() + " " + rval->getL2Token() + " " +
         label->getL2Token();
}
std::string CondJumpInst::getL1Inst() {
  auto newCmpL = lval->getL1Token();
  auto newCmpR = rval->getL1Token();
  auto reversed = true;
  if (dynamic_cast<Number *>(lval) && dynamic_cast<Number *>(rval)) {
    auto cmpL = dynamic_cast<Number *>(lval);
    auto cmpR = dynamic_cast<Number *>(rval);
    // directly compare
    if (op->getID() == CompareOpID::EQUAL && cmpL->getVal() == cmpR->getVal()) {
      return "jmp " + label->getL1Token();
    } else if (op->getID() == CompareOpID::LESS_EQUAL && cmpL->getVal() <= cmpR->getVal()) {
      return "jmp " + label->getL1Token();
    } else if (op->getID() == CompareOpID::LESS_THAN && cmpL->getVal() < cmpR->getVal()) {
      return "jmp " + label->getL1Token();
    } else {
      // don't jump
      return "";
    }
  } else if (dynamic_cast<Number *>(rval)) {
    newCmpL = "$" + rval->getL1Token();
    newCmpR = lval->getL1Token();
    reversed = false;
  } else if (dynamic_cast<Number *>(lval)) {
    newCmpL = '$' + lval->getL1Token();
    newCmpR = rval->getL1Token();
  }
  auto cmpInst = "cmpq " + newCmpL + ", " + newCmpR + "\n  ";

  auto newOp = op->getL1Token();
  if (reversed) {
    if (op->getID() == CompareOpID::LESS_EQUAL) {
      newOp = "ge";
    } else if (op->getID() == CompareOpID::LESS_THAN) {
      newOp = "g";
    }
  }
  auto jmpInst = "j" + newOp + " " + label->getL1Token();
  return cmpInst + jmpInst;
}

} // namespace L1
