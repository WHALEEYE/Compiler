#pragma once

#include <helper.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace L2 {

class Visitor;

class Item {
public:
  virtual std::string toStr() = 0;
  virtual void accept(Visitor &visitor) = 0;
};

class Value : public Item {};

class Symbol : public Value {
public:
  Symbol(std::string name);
  std::string getName();

protected:
  std::string name;
};

class Register : public Symbol {
public:
  enum ID { R8, R9, R10, R11, R12, R13, R14, R15, RAX, RBX, RCX, RDX, RDI, RSI, RBP, RSP };
  static Register *getRegister(ID id);

  std::string toStr() override;
  void accept(Visitor &visitor) override;
  std::string getName8Bit();
  static const std::unordered_set<Register *> &getAllGPRegisters();
  static const std::unordered_set<Register *> &getCallerSavedRegisters();
  static const std::unordered_set<Register *> &getCalleeSavedRegisters();
  static const std::vector<Register *> &getArgRegisters();

private:
  Register(std::string name, std::string name8Bit);
  static const std::unordered_map<ID, Register *> enumMap;

  std::string name8Bit;
  static const std::unordered_set<Register *> allGPRegisters;
  static const std::unordered_set<Register *> callerSavedRegisters;
  static const std::unordered_set<Register *> calleeSavedRegisters;
  static const std::vector<Register *> argRegisters;
};

class Variable : public Symbol {
public:
  Variable(std::string name);
  std::string toStr() override;
  void accept(Visitor &visitor) override;
};

class Number : public Value {
public:
  Number(int64_t val);
  int64_t getVal();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  int64_t val;
};

class CompareOp : public Item {
public:
  enum ID { LESS_THAN, LESS_EQUAL, EQUAL };
  static CompareOp *getCompareOp(ID id);

  std::string getName();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  CompareOp(std::string name);
  static const std::unordered_map<ID, CompareOp *> enumMap;

  std::string name;
};

class ShiftOp : public Item {
public:
  enum ID { LEFT, RIGHT };
  static ShiftOp *getShiftOp(ID id);

  std::string getName();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  ShiftOp(std::string name);
  static const std::unordered_map<ID, ShiftOp *> enumMap;

  std::string name;
};

class ArithOp : public Item {
public:
  enum ID { ADD, SUB, MUL, AND };
  static ArithOp *getArithOp(ID id);

  std::string getName();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  ArithOp(std::string name);
  static const std::unordered_map<ID, ArithOp *> enumMap;

  std::string name;
};

class SelfModOp : public Item {
public:
  enum ID { INC, DEC };
  static SelfModOp *getSelfModOp(ID id);

  std::string getName();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  SelfModOp(std::string name);
  static const std::unordered_map<ID, SelfModOp *> enumMap;

  std::string name;
};

class MemoryLocation : public Item {
public:
  MemoryLocation(Item *base, Number *offset);
  Item *getBase();
  Number *getOffset();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  Item *base;
  Number *offset;
};

class StackLocation : public Item {
public:
  StackLocation(Number *offset);
  Number *getOffset();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  Number *offset;
};

class FunctionName : public Item {
public:
  FunctionName(std::string name);
  std::string getName();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  std::string name;
};

class Label : public Item {
public:
  Label(std::string name);
  std::string getName();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  std::string name;
};

/*
 * Instruction interface.
 */
class Instruction {
public:
  virtual std::string toStr() = 0;
  virtual void accept(Visitor &visitor) = 0;
};

/*
 * Instructions.
 */
class RetInst : public Instruction {
public:
  std::string toStr() override;
  void accept(Visitor &visitor) override;
};

class ShiftInst : public Instruction {
public:
  ShiftInst(ShiftOp *op, Symbol *lval, Value *rval);
  ShiftOp *getOp();
  Symbol *getLval();
  Value *getRval();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  ShiftOp *op;
  Symbol *lval;
  Value *rval;
};

class ArithInst : public Instruction {
public:
  ArithInst(ArithOp *op, Item *lval, Item *rval);
  ArithOp *getOp();
  Item *getLval();
  Item *getRval();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  ArithOp *op;
  Item *lval;
  Item *rval;
};

class SelfModInst : public Instruction {
public:
  SelfModInst(SelfModOp *op, Symbol *lval);
  SelfModOp *getOp();
  Symbol *getLval();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  SelfModOp *op;
  Symbol *lval;
};

class AssignInst : public Instruction {
public:
  AssignInst(Item *lval, Item *rval);
  Item *getLval();
  Item *getRval();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  Item *lval;
  Item *rval;
};

class CompareAssignInst : public Instruction {
public:
  CompareAssignInst(Symbol *lval, CompareOp *op, Value *cmpLval, Value *cmpRval);
  Symbol *getLval();
  CompareOp *getOp();
  Value *getCmpLval();
  Value *getCmpRval();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  Symbol *lval;
  CompareOp *op;
  Value *cmpLval;
  Value *cmpRval;
};

class CallInst : public Instruction {
public:
  CallInst(Item *callee, Number *argNum);
  Item *getCallee();
  Number *getArgNum();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  Item *callee;
  Number *argNum;
};

class PrintInst : public Instruction {
public:
  std::string toStr() override;
  void accept(Visitor &visitor) override;
};

class InputInst : public Instruction {
public:
  std::string toStr() override;
  void accept(Visitor &visitor) override;
};

class AllocateInst : public Instruction {
public:
  std::string toStr() override;
  void accept(Visitor &visitor) override;
};

class TupleErrorInst : public Instruction {
public:
  std::string toStr() override;
  void accept(Visitor &visitor) override;
};

class TensorErrorInst : public Instruction {
public:
  TensorErrorInst(Number *number);
  Number *getArgNum();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  Number *argNum;
};

class SetInst : public Instruction {
public:
  SetInst(Symbol *lval, Symbol *base, Symbol *offset, Number *scalar);
  Symbol *getLval();
  Symbol *getBase();
  Symbol *getOffset();
  Number *getScalar();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  Symbol *lval;
  Symbol *base;
  Symbol *offset;
  Number *scalar;
};

class LabelInst : public Instruction {
public:
  LabelInst(Label *label);
  Label *getLabel();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  Label *label;
};

class GotoInst : public Instruction {
public:
  GotoInst(Label *label);
  Label *getLabel();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  Label *label;
};

class CondJumpInst : public Instruction {
public:
  CondJumpInst(CompareOp *op, Value *lval, Value *rval, Label *label);
  CompareOp *getOp();
  Value *getLval();
  Value *getRval();
  Label *getLabel();
  std::string toStr() override;
  void accept(Visitor &visitor) override;

private:
  CompareOp *op;
  Value *lval;
  Value *rval;
  Label *label;
};

/*
 * Structres.
 */

class SymbolTable {
public:
  void addSymbol(std::string name, Item *item);
  Item *getSymbol(std::string name);

private:
  std::unordered_map<std::string, Item *> table;
  SymbolTable *parent;
};

class BasicBlock {
public:
  const std::vector<Instruction *> &getInstructions();
  void addInstruction(Instruction *inst);
  const std::unordered_set<BasicBlock *> &getPredecessors();
  void addPredecessor(BasicBlock *BB);
  void removePredecessor(BasicBlock *BB);
  const std::unordered_set<BasicBlock *> &getSuccessors();
  void addSuccessor(BasicBlock *BB);
  void removeSuccessor(BasicBlock *BB);
  Instruction *getFirstInstruction();
  Instruction *getTerminator();

private:
  std::vector<Instruction *> instructions;
  std::unordered_set<BasicBlock *> predecessors;
  std::unordered_set<BasicBlock *> successors;
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
  void popCurrBasicBlock();
  Variable *getVariable(std::string name);

private:
  std::string name;
  int64_t paramNum;
  std::vector<BasicBlock *> basicBlocks;
  std::unordered_map<std::string, Variable *> variables;
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

class Visitor {
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
};

} // namespace L2
