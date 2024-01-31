#include "L1.h"
#include <string>

namespace L1 {

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

std::string Item::getL1Token() { return "<unknown-L1-token>"; }
std::string Item::getX86Token() { return "<unknown-x86-token>"; }

Register::Register(RegisterID id) : id{id} { return; }
std::string Register::getL1Token() { return regToken[id]; }
std::string Register::getX86Token() { return "%" + regToken[id]; }
std::string Register::getX86Token8() { return "%" + regToken8[id]; }

Number::Number(int64_t val) : val{val} { return; }
int64_t Number::getVal() { return val; }
std::string Number::getL1Token() { return std::to_string(val); }
std::string Number::getX86Token() { return std::to_string(val); }

CompareOp::CompareOp(CompareOpID id) : id{id} { return; }
CompareOpID CompareOp::getID() { return id; }
std::string CompareOp::getL1Token() { return cmpOpL1Token[id]; }
std::string CompareOp::getX86Token() { return cmpOpX86Token[id]; }

ShiftOp::ShiftOp(ShiftOpID id) : id{id} { return; }
ShiftOpID ShiftOp::getID() { return id; }
std::string ShiftOp::getL1Token() { return shiftOpL1Token[id]; }
std::string ShiftOp::getX86Token() { return shiftOpX86Token[id]; }

ArithOp::ArithOp(ArithOpID id) : id{id} { return; }
ArithOpID ArithOp::getID() { return id; }
std::string ArithOp::getL1Token() { return arithOpL1Token[id]; }
std::string ArithOp::getX86Token() { return arithOpX86Token[id]; }

SelfModOp::SelfModOp(SelfModOpID id) : id{id} { return; }
SelfModOpID SelfModOp::getID() { return id; }
std::string SelfModOp::getL1Token() { return selfModOpL1Token[id]; }
std::string SelfModOp::getX86Token() { return selfModOpX86Token[id]; }

MemoryLocation::MemoryLocation(Register *reg, Number *offset) : reg{reg}, offset{offset} { return; }
Register *MemoryLocation::getReg() { return reg; }
Number *MemoryLocation::getOffset() { return offset; }
std::string MemoryLocation::getL1Token() {
  return "mem " + reg->getL1Token() + " " + offset->getL1Token();
}
std::string MemoryLocation::getX86Token() {
  return offset->getX86Token() + "(" + reg->getX86Token() + ")";
}

FunctionName::FunctionName(std::string name) : pureName{name.substr(1)} { return; }
std::string FunctionName::getPureName() { return pureName; }
std::string FunctionName::getL1Token() { return '@' + pureName; }
std::string FunctionName::getX86Token() { return '_' + pureName; }

Label::Label(std::string name) : pureName{name.substr(1)} { return; }
std::string Label::getPureName() { return pureName; }
std::string Label::getL1Token() { return ':' + pureName; }
std::string Label::getX86Token() { return '_' + pureName; }

/*
 *  Instructions.
 */
std::string Instruction::getL1Inst() { return "<unknown-L1-inst>"; }
std::string Instruction::getX86Inst() { return "<unknown-x86-inst>"; }

std::string RetInst::getL1Inst() { return "return"; }
std::string RetInst::getX86Inst() { return "retq"; }

ShiftInst::ShiftInst(ShiftOp *op, Item *lval, Item *rval) : op{op}, lval{lval}, rval{rval} {
  return;
}
std::string ShiftInst::getL1Inst() {
  return lval->getL1Token() + " " + op->getL1Token() + " " + rval->getL1Token();
}
std::string ShiftInst::getX86Inst() {
  std::string amount = "$" + rval->getX86Token();
  if (dynamic_cast<Register *>(rval)) {
    amount = "%cl";
  }
  return op->getX86Token() + " " + amount + ", " + lval->getX86Token();
}

ArithInst::ArithInst(ArithOp *op, Item *lval, Item *rval) : op{op}, lval{lval}, rval{rval} {
  return;
}
std::string ArithInst::getL1Inst() {
  return lval->getL1Token() + " " + op->getL1Token() + " " + rval->getL1Token();
}
std::string ArithInst::getX86Inst() {
  auto r = rval->getX86Token();
  if (dynamic_cast<Number *>(rval))
    r = "$" + r;
  return op->getX86Token() + " " + r + ", " + lval->getX86Token();
}

SelfModInst::SelfModInst(SelfModOp *op, Item *lval) : op{op}, lval{lval} { return; }
std::string SelfModInst::getL1Inst() { return lval->getL1Token() + " " + op->getL1Token(); }
std::string SelfModInst::getX86Inst() { return op->getX86Token() + " " + lval->getX86Token(); }

AssignInst::AssignInst(Item *lval, Item *rval) : lval{lval}, rval{rval} { return; }
std::string AssignInst::getL1Inst() { return lval->getL1Token() + " <- " + rval->getL1Token(); }
std::string AssignInst::getX86Inst() {
  auto r = rval->getX86Token();
  if (dynamic_cast<FunctionName *>(rval) || dynamic_cast<Number *>(rval) ||
      dynamic_cast<Label *>(rval)) {
    r = '$' + rval->getX86Token();
  }
  return "movq " + r + ", " + lval->getX86Token();
}

CompareAssignInst::CompareAssignInst(Register *lval, CompareOp *op, Item *cmpLval, Item *cmpRval)
    : lval{lval}, op{op}, cmpLval{cmpLval}, cmpRval{cmpRval} {
  return;
}
std::string CompareAssignInst::getL1Inst() {
  return lval->getL1Token() + " <- " + cmpLval->getL1Token() + " " + op->getL1Token() + " " +
         cmpRval->getL1Token();
}
std::string CompareAssignInst::getX86Inst() {
  auto newCmpL = cmpLval->getX86Token();
  auto newCmpR = cmpRval->getX86Token();
  auto reversed = true;
  if (dynamic_cast<Number *>(cmpLval) && dynamic_cast<Number *>(cmpRval)) {
    auto cmpL = dynamic_cast<Number *>(cmpLval);
    auto cmpR = dynamic_cast<Number *>(cmpRval);
    // directly compare
    if (op->getID() == CompareOpID::EQUAL && cmpL->getVal() == cmpR->getVal()) {
      return "movq $1, " + lval->getX86Token();
    } else if (op->getID() == CompareOpID::LESS_EQUAL && cmpL->getVal() <= cmpR->getVal()) {
      return "movq $1, " + lval->getX86Token();
    } else if (op->getID() == CompareOpID::LESS_THAN && cmpL->getVal() < cmpR->getVal()) {
      return "movq $1, " + lval->getX86Token();
    } else {
      return "movq $0, " + lval->getX86Token();
    }
  } else if (dynamic_cast<Number *>(cmpRval)) {
    newCmpL = "$" + cmpRval->getX86Token();
    newCmpR = cmpLval->getX86Token();
    reversed = false;
  } else if (dynamic_cast<Number *>(cmpLval)) {
    newCmpL = '$' + cmpLval->getX86Token();
    newCmpR = cmpRval->getX86Token();
  }
  auto cmpInst = "cmpq " + newCmpL + ", " + newCmpR + "\n  ";

  auto newOp = op->getX86Token();
  if (reversed) {
    if (op->getID() == CompareOpID::LESS_EQUAL) {
      newOp = "ge";
    } else if (op->getID() == CompareOpID::LESS_THAN) {
      newOp = "g";
    }
  }
  auto setInst = "set" + newOp + " " + lval->getX86Token8() + "\n  ";
  auto movInst = "movzbq " + lval->getX86Token8() + ", " + lval->getX86Token();
  return cmpInst + setInst + movInst;
}

CallInst::CallInst(Item *callee, Number *arg_num) : callee{callee}, arg_num{arg_num} { return; }
std::string CallInst::getL1Inst() {
  return "call " + callee->getL1Token() + " " + arg_num->getL1Token();
}
std::string CallInst::getX86Inst() {
  auto jmpInst = "jmp *" + callee->getX86Token();
  if (dynamic_cast<FunctionName *>(callee)) {
    jmpInst = "jmp " + callee->getX86Token();
  }
  auto movAmount = (arg_num->getVal() > 6 ? 8 * (arg_num->getVal() - 6) : 0) + 8;
  auto movRspInst = "subq $" + std::to_string(movAmount) + ", %rsp\n  ";
  return movRspInst + jmpInst;
}

std::string PrintInst::getL1Inst() { return "call print 1"; }
std::string PrintInst::getX86Inst() { return "call print"; }

std::string InputInst::getL1Inst() { return "call input 0"; }
std::string InputInst::getX86Inst() { return "call input"; }

std::string AllocateInst::getL1Inst() { return "call allocate 2"; }
std::string AllocateInst::getX86Inst() { return "call allocate"; }

std::string TupleErrorInst::getL1Inst() { return "call tuple-error 0"; }
std::string TupleErrorInst::getX86Inst() { return "call tuple_error"; }

TensorErrorInst::TensorErrorInst(Number *number) : number(number) { return; }
std::string TensorErrorInst::getL1Inst() { return "call tensor-error " + number->getL1Token(); }
std::string TensorErrorInst::getX86Inst() {
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
std::string SetInst::getL1Inst() {
  return lval->getL1Token() + " @ " + base->getL1Token() + " " + offset->getL1Token() + " " +
         scalar->getL1Token();
}
std::string SetInst::getX86Inst() {
  return "lea (" + base->getX86Token() + ", " + offset->getX86Token() + ", " +
         scalar->getX86Token() + "), " + lval->getX86Token();
}

LabelInst::LabelInst(Label *label) : label{label} { return; }
std::string LabelInst::getL1Inst() { return label->getL1Token(); }
std::string LabelInst::getX86Inst() { return label->getX86Token() + ":"; }

GotoInst::GotoInst(Label *label) : label{label} { return; }
std::string GotoInst::getL1Inst() { return "goto " + label->getL1Token(); }
std::string GotoInst::getX86Inst() { return "jmp " + label->getX86Token(); }

CondJumpInst::CondJumpInst(CompareOp *op, Item *lval, Item *rval, Label *label)
    : op{op}, lval{lval}, rval{rval}, label{label} {
  return;
}
std::string CondJumpInst::getL1Inst() {
  return "cjump " + lval->getL1Token() + " " + op->getL1Token() + " " + rval->getL1Token() + " " +
         label->getL1Token();
}
std::string CondJumpInst::getX86Inst() {
  auto newCmpL = lval->getX86Token();
  auto newCmpR = rval->getX86Token();
  auto reversed = true;
  if (dynamic_cast<Number *>(lval) && dynamic_cast<Number *>(rval)) {
    auto cmpL = dynamic_cast<Number *>(lval);
    auto cmpR = dynamic_cast<Number *>(rval);
    // directly compare
    if (op->getID() == CompareOpID::EQUAL && cmpL->getVal() == cmpR->getVal()) {
      return "jmp " + label->getX86Token();
    } else if (op->getID() == CompareOpID::LESS_EQUAL && cmpL->getVal() <= cmpR->getVal()) {
      return "jmp " + label->getX86Token();
    } else if (op->getID() == CompareOpID::LESS_THAN && cmpL->getVal() < cmpR->getVal()) {
      return "jmp " + label->getX86Token();
    } else {
      // don't jump
      return "";
    }
  } else if (dynamic_cast<Number *>(rval)) {
    newCmpL = "$" + rval->getX86Token();
    newCmpR = lval->getX86Token();
    reversed = false;
  } else if (dynamic_cast<Number *>(lval)) {
    newCmpL = '$' + lval->getX86Token();
    newCmpR = rval->getX86Token();
  }
  auto cmpInst = "cmpq " + newCmpL + ", " + newCmpR + "\n  ";

  auto newOp = op->getX86Token();
  if (reversed) {
    if (op->getID() == CompareOpID::LESS_EQUAL) {
      newOp = "ge";
    } else if (op->getID() == CompareOpID::LESS_THAN) {
      newOp = "g";
    }
  }
  auto jmpInst = "j" + newOp + " " + label->getX86Token();
  return cmpInst + jmpInst;
}

} // namespace L1
