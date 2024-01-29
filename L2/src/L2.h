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
  virtual std::string toStr() const = 0;
  virtual void accept(Visitor &visitor) const = 0;
};

class Value : public Item {};

class Symbol : public Value {
public:
  Symbol(std::string name);
  const std::string getName() const;

protected:
  std::string name;
};

class Register : public Symbol {
public:
  enum ID { R8, R9, R10, R11, R12, R13, R14, R15, RAX, RBX, RCX, RDX, RDI, RSI, RBP, RSP };
  static const Register *getRegister(ID id);

  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
  const std::string getName8Bit() const;
  static const std::unordered_set<const Register *> &getAllGPRegisters();
  static const std::unordered_set<const Register *> &getCallerSavedRegisters();
  static const std::unordered_set<const Register *> &getCalleeSavedRegisters();
  static const std::vector<const Register *> &getArgRegisters();

private:
  Register(std::string name, std::string name8Bit);
  static const std::unordered_map<ID, const Register *> enumMap;

  std::string name8Bit;
  static const std::unordered_set<const Register *> allGPRegisters;
  static const std::unordered_set<const Register *> callerSavedRegisters;
  static const std::unordered_set<const Register *> calleeSavedRegisters;
  static const std::vector<const Register *> argRegisters;
};

class Variable : public Symbol {
public:
  Variable(std::string name);
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
};

class Number : public Value {
public:
  Number(int64_t val);
  int64_t getVal() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  int64_t val;
};

class CompareOp : public Item {
public:
  enum ID { LESS_THAN, LESS_EQUAL, EQUAL };
  static CompareOp *getCompareOp(ID id);

  const std::string getName() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  CompareOp(std::string name);
  static const std::unordered_map<ID, CompareOp *> enumMap;

  std::string name;
};

class ShiftOp : public Item {
public:
  enum ID { LEFT, RIGHT };
  static ShiftOp *getShiftOp(ID id);

  const std::string getName() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  ShiftOp(std::string name);
  static const std::unordered_map<ID, ShiftOp *> enumMap;

  std::string name;
};

class ArithOp : public Item {
public:
  enum ID { ADD, SUB, MUL, AND };
  static ArithOp *getArithOp(ID id);

  const std::string getName() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  ArithOp(std::string name);
  static const std::unordered_map<ID, ArithOp *> enumMap;

  std::string name;
};

class SelfModOp : public Item {
public:
  enum ID { INC, DEC };
  static SelfModOp *getSelfModOp(ID id);

  const std::string getName() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  SelfModOp(std::string name);
  static const std::unordered_map<ID, SelfModOp *> enumMap;

  std::string name;
};

class MemoryLocation : public Item {
public:
  MemoryLocation(const Symbol *base, const Number *offset);
  const Symbol *getBase() const;
  const Number *getOffset() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Symbol *base;
  const Number *offset;
};

class StackLocation : public Item {
public:
  StackLocation(const Number *offset);
  const Number *getOffset() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Number *offset;
};

class FunctionName : public Item {
public:
  FunctionName(std::string name);
  std::string getName() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  std::string name;
};

class Label : public Item {
public:
  Label(std::string name);
  std::string getName() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  std::string name;
};

/*
 * Instruction interface.
 */
class Instruction {
public:
  virtual std::string toStr() const = 0;
  virtual void accept(Visitor &visitor) const = 0;
};

/*
 * Instructions.
 */
class RetInst : public Instruction {
public:
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
};

class ShiftInst : public Instruction {
public:
  ShiftInst(const ShiftOp *op, const Symbol *lval, const Value *rval);
  const ShiftOp *getOp() const;
  const Symbol *getLval() const;
  const Value *getRval() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const ShiftOp *op;
  const Symbol *lval;
  const Value *rval;
};

class ArithInst : public Instruction {
public:
  ArithInst(const ArithOp *op, const Item *lval, const Item *rval);
  const ArithOp *getOp() const;
  const Item *getLval() const;
  const Item *getRval() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const ArithOp *op;
  const Item *lval;
  const Item *rval;
};

class SelfModInst : public Instruction {
public:
  SelfModInst(const SelfModOp *op, const Symbol *lval);
  const SelfModOp *getOp() const;
  const Symbol *getLval() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const SelfModOp *op;
  const Symbol *lval;
};

class AssignInst : public Instruction {
public:
  AssignInst(const Item *lval, const Item *rval);
  const Item *getLval() const;
  const Item *getRval() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Item *lval;
  const Item *rval;
};

class CompareAssignInst : public Instruction {
public:
  CompareAssignInst(const Symbol *lval, const CompareOp *op, const Value *cmpLval,
                    const Value *cmpRval);
  const Symbol *getLval() const;
  const CompareOp *getOp() const;
  const Value *getCmpLval() const;
  const Value *getCmpRval() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Symbol *lval;
  const CompareOp *op;
  const Value *cmpLval;
  const Value *cmpRval;
};

class CallInst : public Instruction {
public:
  CallInst(const Item *callee, const Number *argNum);
  const Item *getCallee() const;
  const Number *getArgNum() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Item *callee;
  const Number *argNum;
};

class PrintInst : public Instruction {
public:
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
};

class InputInst : public Instruction {
public:
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
};

class AllocateInst : public Instruction {
public:
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
};

class TupleErrorInst : public Instruction {
public:
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
};

class TensorErrorInst : public Instruction {
public:
  TensorErrorInst(const Number *number);
  const Number *getArgNum() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Number *argNum;
};

class SetInst : public Instruction {
public:
  SetInst(const Symbol *lval, const Symbol *base, const Symbol *offset, const Number *scalar);
  const Symbol *getLval() const;
  const Symbol *getBase() const;
  const Symbol *getOffset() const;
  const Number *getScalar() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Symbol *lval;
  const Symbol *base;
  const Symbol *offset;
  const Number *scalar;
};

class LabelInst : public Instruction {
public:
  LabelInst(const Label *label);
  const Label *getLabel() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Label *label;
};

class GotoInst : public Instruction {
public:
  GotoInst(const Label *label);
  const Label *getLabel() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Label *label;
};

class CondJumpInst : public Instruction {
public:
  CondJumpInst(const CompareOp *op, const Value *lval, const Value *rval, const Label *label);
  const CompareOp *getOp() const;
  const Value *getLval() const;
  const Value *getRval() const;
  const Label *getLabel() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const CompareOp *op;
  const Value *lval;
  const Value *rval;
  const Label *label;
};

/*
 * Structres.
 */
class BasicBlock {
public:
  const std::vector<const Instruction *> &getInstructions() const;
  void addInstruction(const Instruction *inst);
  const std::unordered_set<BasicBlock *> &getPredecessors() const;
  void addPredecessor(BasicBlock *BB);
  void removePredecessor(BasicBlock *BB);
  const std::unordered_set<BasicBlock *> &getSuccessors() const;
  void addSuccessor(BasicBlock *BB);
  void removeSuccessor(BasicBlock *BB);
  const Instruction *getFirstInstruction() const;
  const Instruction *getTerminator() const;

private:
  std::vector<const Instruction *> instructions;
  std::unordered_set<BasicBlock *> predecessors;
  std::unordered_set<BasicBlock *> successors;

  friend void spillInBB(BasicBlock *BB);
};

class Function {
public:
  Function(std::string name);
  std::string getName();
  int64_t getParamNum();
  void setParameters(int64_t parameters);
  const std::vector<BasicBlock *> &getBasicBlocks() const;
  void addBasicBlock(BasicBlock *BB);
  BasicBlock *getCurrBasicBlock() const;
  void popCurrBasicBlock();
  const Variable *getVariable(std::string name);
  bool hasVariable(std::string name) const;

private:
  std::string name;
  int64_t paramNum;
  std::vector<BasicBlock *> basicBlocks;
  std::unordered_map<std::string, Variable *> variables;
};

class Program {
public:
  Program() = default;
  std::string getEntryPointLabel() const;
  void setEntryPointLabel(std::string label);
  const std::vector<Function *> &getFunctions() const;
  void addFunction(Function *F);
  Function *getCurrFunction() const;

protected:
  std::string entryPointLabel;

private:
  Program(const Program &) = delete;
  Program &operator=(const Program &) = delete;
  std::vector<Function *> functions;
};

class Visitor {
public:
  virtual void visit(const Register *reg) = 0;
  virtual void visit(const Variable *var) = 0;
  virtual void visit(const Number *num) = 0;
  virtual void visit(const CompareOp *op) = 0;
  virtual void visit(const ShiftOp *op) = 0;
  virtual void visit(const ArithOp *op) = 0;
  virtual void visit(const SelfModOp *op) = 0;
  virtual void visit(const MemoryLocation *mem) = 0;
  virtual void visit(const StackLocation *stack) = 0;
  virtual void visit(const FunctionName *name) = 0;
  virtual void visit(const Label *label) = 0;
  virtual void visit(const RetInst *inst) = 0;
  virtual void visit(const ShiftInst *inst) = 0;
  virtual void visit(const ArithInst *inst) = 0;
  virtual void visit(const SelfModInst *inst) = 0;
  virtual void visit(const AssignInst *inst) = 0;
  virtual void visit(const CompareAssignInst *inst) = 0;
  virtual void visit(const CallInst *inst) = 0;
  virtual void visit(const PrintInst *inst) = 0;
  virtual void visit(const InputInst *inst) = 0;
  virtual void visit(const AllocateInst *inst) = 0;
  virtual void visit(const TupleErrorInst *inst) = 0;
  virtual void visit(const TensorErrorInst *inst) = 0;
  virtual void visit(const SetInst *inst) = 0;
  virtual void visit(const LabelInst *inst) = 0;
  virtual void visit(const GotoInst *inst) = 0;
  virtual void visit(const CondJumpInst *inst) = 0;
};

} // namespace L2
