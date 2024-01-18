#pragma once

#include <string>
#include <vector>

namespace L2 {

enum RegisterID { R8, R9, R10, R11, R12, R13, R14, R15, RAX, RBX, RCX, RDX, RDI, RSI, RBP, RSP };

enum CompareOpID { LESS_THAN, LESS_EQUAL, EQUAL };

enum ShiftOpID { LEFT, RIGHT };

enum ArithOpID { ADD, SUB, MUL, AND };

enum SelfModOpID { INC, DEC };

class Item {
public:
  virtual std::string getL2Token();
  virtual std::string getL1Token();
};

class Register : public Item {
public:
  Register(RegisterID id);
  RegisterID getID();
  std::string getL2Token() override;
  std::string getL1Token() override;
  std::string getX86Token8();

private:
  RegisterID id;
};

class Variable : public Item {
public:
  Variable(std::string name);
  std::string getPureName();
  std::string getL2Token() override;
  std::string getL1Token() override;

private:
  std::string pureName;
};

class Number : public Item {
public:
  Number(int64_t val);
  int64_t getVal();
  std::string getL2Token() override;
  std::string getL1Token() override;

private:
  int64_t val;
};

class CompareOp : public Item {
public:
  CompareOp(CompareOpID id);
  CompareOpID getID();
  std::string getL2Token() override;
  std::string getL1Token() override;

private:
  CompareOpID id;
};

class ShiftOp : public Item {
public:
  ShiftOp(ShiftOpID id);
  ShiftOpID getID();
  std::string getL2Token() override;
  std::string getL1Token() override;

private:
  ShiftOpID id;
};

class ArithOp : public Item {
public:
  ArithOp(ArithOpID id);
  ArithOpID getID();
  std::string getL2Token() override;
  std::string getL1Token() override;

private:
  ArithOpID id;
};

class SelfModOp : public Item {
public:
  SelfModOp(SelfModOpID id);
  SelfModOpID getID();
  std::string getL2Token() override;
  std::string getL1Token() override;

private:
  SelfModOpID id;
};

class MemoryLocation : public Item {
public:
  MemoryLocation(Register *reg, Number *offset);
  Register *getReg();
  Number *getOffset();
  std::string getL2Token() override;
  std::string getL1Token() override;

private:
  Register *reg;
  Number *offset;
};

class StackLocation : public Item {
public:
  StackLocation(Number *offset);
  Number *getOffset();
  std::string getL2Token() override;
  std::string getL1Token() override;

private:
  Number *offset;
};

class FunctionName : public Item {
public:
  FunctionName(std::string name);
  std::string getPureName();
  std::string getL2Token() override;
  std::string getL1Token() override;

private:
  std::string pureName;
};

class Label : public Item {
public:
  Label(std::string name);
  std::string getPureName();
  std::string getL2Token() override;
  std::string getL1Token() override;

private:
  std::string pureName;
};

/*
 * Instruction interface.
 */
class Instruction {
public:
  virtual std::string getL2Inst();
  virtual std::string getL1Inst();
};

/*
 * Instructions.
 */
class RetInst : public Instruction {
public:
  std::string getL2Inst() override;
  std::string getL1Inst() override;
};

class ShiftInst : public Instruction {
public:
  ShiftInst(ShiftOp *op, Item *lval, Item *rval);
  ShiftOp *getOp();
  Item *getLval();
  Item *getRval();
  std::string getL2Inst() override;
  std::string getL1Inst() override;

private:
  ShiftOp *op;
  Item *lval;
  Item *rval;
};

class ArithInst : public Instruction {
public:
  ArithInst(ArithOp *op, Item *lval, Item *rval);
  ArithOp *getOp();
  Item *getLval();
  Item *getRval();
  std::string getL2Inst() override;
  std::string getL1Inst() override;

private:
  ArithOp *op;
  Item *lval;
  Item *rval;
};

class SelfModInst : public Instruction {
public:
  SelfModInst(SelfModOp *op, Item *lval);
  SelfModOp *getOp();
  Item *getLval();
  std::string getL2Inst() override;
  std::string getL1Inst() override;

private:
  SelfModOp *op;
  Item *lval;
};

class AssignInst : public Instruction {
public:
  AssignInst(Item *lval, Item *rval);
  Item *getLval();
  Item *getRval();
  std::string getL2Inst() override;
  std::string getL1Inst() override;

private:
  Item *lval;
  Item *rval;
};

class CompareAssignInst : public Instruction {
public:
  CompareAssignInst(Register *lval, CompareOp *op, Item *cmpLval, Item *cmpRval);
  Register *getLval();
  CompareOp *getOp();
  Item *getCmpLval();
  Item *getCmpRval();
  std::string getL2Inst() override;
  std::string getL1Inst() override;

private:
  Register *lval;
  CompareOp *op;
  Item *cmpLval;
  Item *cmpRval;
};

class CallInst : public Instruction {
public:
  CallInst(Item *callee, Number *arg_num);
  Item *getCallee();
  Number *getArgNum();
  std::string getL2Inst() override;
  std::string getL1Inst() override;

private:
  Item *callee;
  Number *arg_num;
};

class PrintInst : public Instruction {
public:
  std::string getL2Inst() override;
  std::string getL1Inst() override;
};

class InputInst : public Instruction {
public:
  std::string getL2Inst() override;
  std::string getL1Inst() override;
};

class AllocateInst : public Instruction {
public:
  std::string getL2Inst() override;
  std::string getL1Inst() override;
};

class TupleErrorInst : public Instruction {
public:
  std::string getL2Inst() override;
  std::string getL1Inst() override;
};

class TensorErrorInst : public Instruction {
public:
  TensorErrorInst(Number *number);
  Number *getNumber();
  std::string getL2Inst() override;
  std::string getL1Inst() override;

private:
  Number *number;
};

class SetInst : public Instruction {
public:
  SetInst(Register *lval, Register *base, Register *offset, Number *scalar);
  Register *getLval();
  Register *getBase();
  Register *getOffset();
  Number *getScalar();
  std::string getL2Inst() override;
  std::string getL1Inst() override;

private:
  Register *lval;
  Register *base;
  Register *offset;
  Number *scalar;
};

class LabelInst : public Instruction {
public:
  LabelInst(Label *label);
  Label *getLabel();
  std::string getL2Inst() override;
  std::string getL1Inst() override;

private:
  Label *label;
};

class GotoInst : public Instruction {
public:
  GotoInst(Label *label);
  Label *getLabel();
  std::string getL2Inst() override;
  std::string getL1Inst() override;

private:
  Label *label;
};

class CondJumpInst : public Instruction {
public:
  CondJumpInst(CompareOp *op, Item *lval, Item *rval, Label *label);
  CompareOp *getOp();
  Item *getLval();
  Item *getRval();
  Label *getLabel();
  std::string getL2Inst() override;
  std::string getL1Inst() override;

private:
  CompareOp *op;
  Item *lval;
  Item *rval;
  Label *label;
};

/*
 * Function.
 */
class Function {
public:
  std::string name;
  int64_t parameters;
  std::vector<Instruction *> instructions;
};

class Program {
public:
  std::string entryPointLabel;
  std::vector<Function *> functions;
};

} // namespace L1
