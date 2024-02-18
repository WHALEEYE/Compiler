#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace L3 {

class Visitor;
class Value;

class Item {
public:
  virtual std::string toStr() const;
  virtual void accept(Visitor &visitor) const;
};

class Type : public Item {};

class Int64Type : public Type {
public:
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

  Int64Type(const Int64Type &) = delete;
  Int64Type &operator=(const Int64Type &) = delete;
  static Int64Type *getInstance();

private:
  Int64Type() = default;
  static Int64Type *instance;
};

class ArrayType : public Type {
public:
  void increaseDim();
  int64_t getDim() const;
  void setSizes(const std::vector<const Value *> &sizes);
  const std::vector<const Value *> &getSizes() const;

  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  int64_t dim;
  std::vector<const Value *> sizes;
};

class TupleType : public Type {
public:
  void setSize(const Value *size);
  const Value *getSize() const;

  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Value *size;
};

class CodeType : public Type {
public:
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

  CodeType(const CodeType &) = delete;
  CodeType &operator=(const CodeType &) = delete;

  static CodeType *getInstance();

private:
  CodeType() = default;
  static CodeType *instance;
};

class VoidType : public Type {
public:
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

  VoidType(const VoidType &) = delete;
  VoidType &operator=(const VoidType &) = delete;
  static VoidType *getInstance();

private:
  VoidType() = default;
  static VoidType *instance;
};

class Value : public Item {};

class Variable : public Value {
public:
  Variable(const std::string &name, const Type *type);
  const std::string &getName() const;
  const Type *getType() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Type *type{};
  const std::string name;
};

class Number : public Value {
public:
  explicit Number(int64_t val);
  int64_t getValue() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  int64_t value;
};

class MemoryLocation : public Value {
public:
  MemoryLocation(const Variable *base);
  const Variable *getBase() const;
  void addIndex(const Value *index);
  const std::vector<const Value *> &getIndices() const;

  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *base;
  std::vector<const Value *> indices;
};

class LeftParen : public Item {
public:
  static LeftParen *getInstance();

private:
  LeftParen() = default;
  static LeftParen *instance;
};

class RightParen : public Item {
public:
  static RightParen *getInstance();

private:
  RightParen() = default;
  static RightParen *instance;
};

class Arguments : public Item {
public:
  void addArgToHead(const Value *arg);
  void addArgToTail(const Value *arg);
  const std::vector<const Value *> &getArgs() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  std::vector<const Value *> args;
};

class Parameters : public Item {
public:
  Parameters() = default;
  void addParamToHead(const Variable *param);
  const std::vector<const Variable *> &getParams() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  std::vector<const Variable *> params;
};

class CompareOp : public Item {
public:
  enum ID { LESS_THAN, LESS_EQUAL, EQUAL, GREATER_EQUAL, GREATER_THAN };
  static CompareOp *getCompareOp(ID id);

  std::string getName() const;
  ID getID() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  CompareOp(ID id, std::string name);
  static const std::unordered_map<ID, CompareOp *> enumMap;

  const ID id;
  std::string name;
};

class ArithOp : public Item {
public:
  enum ID { ADD, SUB, MUL, AND, LS, RS };
  static ArithOp *getArithOp(ID id);

  std::string getName() const;
  ID getID() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  ArithOp(ID id, std::string name);
  static const std::unordered_map<ID, ArithOp *> enumMap;

  const ID id;
  std::string name;
};

class RuntimeFunction : public Item {
public:
  enum ID { PRINT, ALLOCATE, INPUT, TUPLE_ERROR, TENSOR_ERROR };
  static RuntimeFunction *getRuntimeFunction(ID id);

  std::string getName() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  explicit RuntimeFunction(std::string name);
  static const std::unordered_map<ID, RuntimeFunction *> enumMap;

  std::string name;
};

class FunctionName : public Item {
public:
  explicit FunctionName(std::string name);
  std::string getName() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  std::string name;
};

class Label : public Item {
public:
  explicit Label(std::string name);
  std::string getName() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  std::string name;
  friend void renameLabel(Label *label, std::string newName);
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
class DeclarationInst : public Instruction {
public:
  DeclarationInst(const Variable *var);
  const Variable *getVar() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *var;
};

class AssignInst : public Instruction {
public:
  AssignInst(const Variable *lval, const Item *rval);
  const Variable *getLhs() const;
  const Item *getRhs() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *lhs;
  const Item *rhs;
};

class ArithInst : public Instruction {
public:
  ArithInst(const Variable *lval, const Value *arithLval, const ArithOp *op, const Value *arithRval);
  const Variable *getRst() const;
  const Value *getLhs() const;
  const ArithOp *getOp() const;
  const Value *getRhs() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *rst;
  const Value *lhs;
  const ArithOp *op;
  const Value *rhs;
};

class CompareInst : public Instruction {
public:
  CompareInst(const Variable *lval, const Value *cmpLval, const CompareOp *op, const Value *cmpRval);
  const Variable *getRst() const;
  const Value *getLhs() const;
  const CompareOp *getOp() const;
  const Value *getRhs() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *rst;
  const Value *lhs;
  const CompareOp *op;
  const Value *rhs;
};

class LoadInst : public Instruction {
public:
  LoadInst(const Variable *target, const MemoryLocation *memLoc);
  const Variable *getTarget() const;
  const MemoryLocation *getMemLoc() const;

  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *target;
  const MemoryLocation *memLoc;
};

class StoreInst : public Instruction {
public:
  StoreInst(const MemoryLocation *memLoc, const Value *source);
  const MemoryLocation *getMemLoc() const;
  const Value *getSource() const;

  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const MemoryLocation *memLoc;
  const Value *source;
};

class ArrayLenInst : public Instruction {
public:
  ArrayLenInst(const Variable *result, const Variable *base, const Value *dimIndex);
  const Variable *getResult() const;
  const Variable *getBase() const;
  const Value *getDimIndex() const;

  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *result;
  const Variable *base;
  const Value *dimIndex;
};

class TupleLenInst : public Instruction {
public:
  TupleLenInst(const Variable *result, const Variable *base);
  const Variable *getResult() const;
  const Variable *getBase() const;

  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *result;
  const Variable *base;
};

class NewArrayInst : public Instruction {
public:
  NewArrayInst(const Variable *array);
  const Variable *getArray() const;

  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *array;
};

class NewTupleInst : public Instruction {
public:
  NewTupleInst(const Variable *tuple);
  const Variable *getTuple() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *tuple;
};

class RetInst : public Instruction {
public:
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
};

class RetValueInst : public Instruction {
public:
  explicit RetValueInst(const Value *val);
  const Value *getVal() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Value *val;
};

class LabelInst : public Instruction {
public:
  explicit LabelInst(const Label *label);
  const Label *getLabel() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Label *label;
};

class BranchInst : public Instruction {
public:
  explicit BranchInst(const Label *label);
  const Label *getLabel() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Label *label;
};

class CondBranchInst : public Instruction {
public:
  CondBranchInst(const Value *condition, const Label *trueLabel, const Label *falseLabel);
  const Value *getCondition() const;
  const Label *getTrueLabel() const;
  const Label *getFalseLabel() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Value *condition;
  const Label *trueLabel, *falseLabel;
};

class CallInst : public Instruction {
public:
  CallInst(const Item *callee, const Arguments *args);
  const Item *getCallee() const;
  const Arguments *getArgs() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Item *callee;
  const Arguments *args;
};

class CallAssignInst : public Instruction {
public:
  CallAssignInst(const Variable *lval, const Item *callee, const Arguments *args);
  const Variable *getRst() const;
  const Item *getCallee() const;
  const Arguments *getArgs() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *rst;
  const Item *callee;
  const Arguments *args;
};

/*
 * Structures.
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
  bool empty() const;
  std::string toStr() const;

private:
  std::vector<const Instruction *> instructions;
  std::unordered_set<BasicBlock *> predecessors;
  std::unordered_set<BasicBlock *> successors;
};

class Function {
public:
  explicit Function();

  std::string getName() const;
  void setName(const std::string &name);

  const Type *getReturnType() const;
  void setReturnType(const Type *type);

  const Parameters *getParams() const;
  void setParams(const Parameters *parameters);

  void defineVariable(const std::string &name, const Type *type);
  Variable *getVariable(const std::string &name);
  Label *getLabel(const std::string &name);
  const std::unordered_map<std::string, Label *> &getLabels() const;

  void addInstruction(Instruction *inst);

  void newBasicBlock();
  const std::vector<BasicBlock *> &getBasicBlocks() const;

  std::string toStr() const;

private:
  std::string name;
  const Type *returnType{};
  const Parameters *params{};
  std::vector<BasicBlock *> basicBlocks;
  std::unordered_map<std::string, Variable *> variables;
  // the name of a label may need to be changed later
  std::unordered_map<std::string, Label *> labels;
};

class Program {
public:
  Program() = default;
  const std::vector<Function *> &getFunctions() const;
  void addFunction(Function *F);
  Function *getCurrFunction() const;
  void addInstruction(Instruction *inst);

  void defineVariable(const std::string &name, const Type *type);
  Variable *getVariable(const std::string &name) const;
  Label *getLabel(const std::string &name) const;

  void newBasicBlock() const;
  std::string toStr() const;

  Program(const Program &) = delete;
  Program &operator=(const Program &) = delete;

private:
  std::vector<Function *> functions;
};

class Visitor {
public:
  virtual void visit(const Variable *var) = 0;
  virtual void visit(const Number *num) = 0;
  virtual void visit(const MemoryLocation *mem) = 0;
  virtual void visit(const Int64Type *type) = 0;
  virtual void visit(const ArrayType *type) = 0;
  virtual void visit(const TupleType *type) = 0;
  virtual void visit(const CodeType *type) = 0;
  virtual void visit(const VoidType *type) = 0;
  virtual void visit(const Arguments *args) = 0;
  virtual void visit(const Parameters *params) = 0;
  virtual void visit(const CompareOp *op) = 0;
  virtual void visit(const ArithOp *op) = 0;
  virtual void visit(const RuntimeFunction *func) = 0;
  virtual void visit(const FunctionName *name) = 0;
  virtual void visit(const Label *label) = 0;
  virtual void visit(const DeclarationInst *inst) = 0;
  virtual void visit(const AssignInst *inst) = 0;
  virtual void visit(const ArithInst *inst) = 0;
  virtual void visit(const CompareInst *inst) = 0;
  virtual void visit(const LoadInst *inst) = 0;
  virtual void visit(const StoreInst *inst) = 0;
  virtual void visit(const ArrayLenInst *inst) = 0;
  virtual void visit(const TupleLenInst *inst) = 0;
  virtual void visit(const NewArrayInst *inst) = 0;
  virtual void visit(const NewTupleInst *inst) = 0;
  virtual void visit(const RetInst *inst) = 0;
  virtual void visit(const RetValueInst *inst) = 0;
  virtual void visit(const LabelInst *inst) = 0;
  virtual void visit(const BranchInst *inst) = 0;
  virtual void visit(const CondBranchInst *inst) = 0;
  virtual void visit(const CallInst *inst) = 0;
  virtual void visit(const CallAssignInst *inst) = 0;
};

} // namespace L3
