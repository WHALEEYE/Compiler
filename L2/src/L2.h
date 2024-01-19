#pragma once

#include <string>
#include <vector>

#include <helper.h>

namespace L2 {

enum RegisterID { R8, R9, R10, R11, R12, R13, R14, R15, RAX, RBX, RCX, RDX, RDI, RSI, RBP, RSP };

enum CompareOpID { LESS_THAN, LESS_EQUAL, EQUAL };

enum ShiftOpID { LEFT, RIGHT };

enum ArithOpID { ADD, SUB, MUL, AND };

enum SelfModOpID { INC, DEC };

class Item {
public:
  virtual std::string toStr() = 0;
};

class Register : public Item {
public:
  Register(RegisterID id);
  RegisterID getID();
  std::string toStr() override;
  std::string getName();
  std::string get8BitName();

private:
  RegisterID id;
};

class Variable : public Item {
public:
  Variable(std::string name);
  std::string getName();
  std::string toStr() override;

private:
  std::string name;
};

class Number : public Item {
public:
  Number(int64_t val);
  int64_t getVal();
  std::string toStr() override;

private:
  int64_t val;
};

class CompareOp : public Item {
public:
  CompareOp(CompareOpID id);
  CompareOpID getID();
  std::string toStr() override;

private:
  CompareOpID id;
};

class ShiftOp : public Item {
public:
  ShiftOp(ShiftOpID id);
  ShiftOpID getID();
  std::string toStr() override;

private:
  ShiftOpID id;
};

class ArithOp : public Item {
public:
  ArithOp(ArithOpID id);
  ArithOpID getID();
  std::string toStr() override;

private:
  ArithOpID id;
};

class SelfModOp : public Item {
public:
  SelfModOp(SelfModOpID id);
  SelfModOpID getID();
  std::string toStr() override;

private:
  SelfModOpID id;
};

class MemoryLocation : public Item {
public:
  MemoryLocation(Register *reg, Number *offset);
  Register *getReg();
  Number *getOffset();
  std::string toStr() override;

private:
  Register *reg;
  Number *offset;
};

class StackLocation : public Item {
public:
  StackLocation(Number *offset);
  Number *getOffset();
  std::string toStr() override;

private:
  Number *offset;
};

class FunctionName : public Item {
public:
  FunctionName(std::string name);
  std::string getName();
  std::string toStr() override;

private:
  std::string name;
};

class Label : public Item {
public:
  Label(std::string name);
  std::string getName();
  std::string toStr() override;

private:
  std::string name;
};

/*
 * Instruction interface.
 */
class Instruction {
public:
  virtual std::string toStr() = 0;
};

/*
 * Instructions.
 */
class RetInst : public Instruction {
public:
  std::string toStr() override;
};

class ShiftInst : public Instruction {
public:
  ShiftInst(ShiftOp *op, Item *lval, Item *rval);
  ShiftOp *getOp();
  Item *getLval();
  Item *getRval();
  std::string toStr() override;

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
  std::string toStr() override;

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
  std::string toStr() override;

private:
  SelfModOp *op;
  Item *lval;
};

class AssignInst : public Instruction {
public:
  AssignInst(Item *lval, Item *rval);
  Item *getLval();
  Item *getRval();
  std::string toStr() override;

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
  std::string toStr() override;

private:
  Register *lval;
  CompareOp *op;
  Item *cmpLval;
  Item *cmpRval;
};

class CallInst : public Instruction {
public:
  CallInst(Item *callee, Number *argNum);
  Item *getCallee();
  Number *getArgNum();
  std::string toStr() override;

private:
  Item *callee;
  Number *argNum;
};

class PrintInst : public Instruction {
public:
  std::string toStr() override;
};

class InputInst : public Instruction {
public:
  std::string toStr() override;
};

class AllocateInst : public Instruction {
public:
  std::string toStr() override;
};

class TupleErrorInst : public Instruction {
public:
  std::string toStr() override;
};

class TensorErrorInst : public Instruction {
public:
  TensorErrorInst(Number *number);
  Number *getNumber();
  std::string toStr() override;

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
  std::string toStr() override;

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
  std::string toStr() override;

private:
  Label *label;
};

class GotoInst : public Instruction {
public:
  GotoInst(Label *label);
  Label *getLabel();
  std::string toStr() override;

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
  std::string toStr() override;

private:
  CompareOp *op;
  Item *lval;
  Item *rval;
  Label *label;
};

/*
 * Structres.
 */
class BasicBlock {
public:
  const std::vector<Instruction *> &getInstructions();
  void addInstruction(Instruction *inst);
  const std::vector<BasicBlock *> &getPredecessors();
  void addPredecessor(BasicBlock *BB);
  const std::vector<BasicBlock *> &getSuccessors();
  void addSuccessor(BasicBlock *BB);
  Instruction *getFirstInstruction();
  Instruction *getTerminator();

private:
  std::vector<Instruction *> instructions;
  std::vector<BasicBlock *> predecessors;
  std::vector<BasicBlock *> successors;
};

class Function {
public:
  Function(std::string name);
  std::string getName();
  int64_t getParamNum();
  void setParameters(int64_t parameters);
  const std::vector<BasicBlock *> &getBasicBlocks();
  void addBasicBlock(BasicBlock *BB);
  BasicBlock *getCurrBasicBlock();

private:
  std::string name;
  int64_t paramNum;
  std::vector<BasicBlock *> basicBlocks;
};

class Program {
public:
  std::string getEntryPointLabel();
  void setEntryPointLabel(std::string label);
  const std::vector<Function *> &getFunctions();
  void addFunction(Function *F);
  Function *getCurrFunction();

private:
  std::string entryPointLabel;
  std::vector<Function *> functions;
};

class L2Visitor {
public:
  virtual void visit(Register *reg) = 0;
  virtual void visit(Variable *var) = 0;
  virtual void visit(Number *num) = 0;
  virtual void visit(CompareOp *op) = 0;
  virtual void visit(ShiftOp *op) = 0;
  virtual void visit(ArithOp *op) = 0;
  virtual void visit(SelfModOp *op) = 0;
  virtual void visit(MemoryLocation *mem) = 0;
  virtual void visit(StackLocation *stack) = 0;
  virtual void visit(FunctionName *name) = 0;
  virtual void visit(Label *label) = 0;
  virtual void visit(RetInst *inst) = 0;
  virtual void visit(ShiftInst *inst) = 0;
  virtual void visit(ArithInst *inst) = 0;
  virtual void visit(SelfModInst *inst) = 0;
  virtual void visit(AssignInst *inst) = 0;
  virtual void visit(CompareAssignInst *inst) = 0;
  virtual void visit(CallInst *inst) = 0;
  virtual void visit(PrintInst *inst) = 0;
  virtual void visit(InputInst *inst) = 0;
  virtual void visit(AllocateInst *inst) = 0;
  virtual void visit(TupleErrorInst *inst) = 0;
  virtual void visit(TensorErrorInst *inst) = 0;
  virtual void visit(SetInst *inst) = 0;
  virtual void visit(LabelInst *inst) = 0;
  virtual void visit(GotoInst *inst) = 0;
  virtual void visit(CondJumpInst *inst) = 0;
  virtual void visit(Function *func) = 0;
  virtual void visit(Program *prog) = 0;
};

} // namespace L2
