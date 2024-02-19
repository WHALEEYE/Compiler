#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace L3 {

class Program;

class Visitor;

class Item {
public:
  virtual std::string toStr() const;
  virtual void accept(Visitor &visitor) const;
};

class Value : public Item {};

class Variable : public Value {
public:
  std::string getName() const;
  explicit Variable(std::string name);
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  std::string name;
};

class Number : public Value {
public:
  explicit Number(int64_t val);
  int64_t getVal() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  int64_t val;
};

class LeftParen : public Item {
public:
  static const LeftParen *getInstance();

private:
  LeftParen() = default;
  static const LeftParen *instance;
};

class RightParen : public Item {
public:
  static const RightParen *getInstance();

private:
  RightParen() = default;
  static const RightParen *instance;
};

class Arguments : public Item {
public:
  Arguments() = default;
  void addArgToHead(const Value *arg);
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
class Context;

class Instruction {
public:
  virtual std::string toStr() const = 0;
  virtual void accept(Visitor &visitor) const = 0;
  void setContext(const Context *context);
  const Context *getContext() const;

private:
  const Context *cxt{};
};

/*
 * Instructions.
 */
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
  LoadInst(const Variable *lval, const Variable *addr);
  const Variable *getVal() const;
  const Variable *getAddr() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *val;
  const Variable *addr;
};

class StoreInst : public Instruction {
public:
  StoreInst(const Variable *addr, const Value *rval);
  const Variable *getAddr() const;
  const Value *getVal() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *addr;
  const Value *val;
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
  CondBranchInst(const Value *condition, const Label *label);
  const Value *getCondition() const;
  const Label *getLabel() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Value *condition;
  const Label *label;
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

class Context {
public:
  const std::vector<const Instruction *> &getInstructions() const;
  void addInstruction(const Instruction *inst);

private:
  std::vector<const Instruction *> instructions;
};

class Function {
public:
  explicit Function(std::string name);
  std::string getName() const;
  const Parameters *getParams() const;
  void setParams(const Parameters *parameters);
  const Variable *getVariable(const std::string &name);
  bool hasVariable(const std::string &name) const;
  const std::unordered_map<std::string, const Variable *> &getVariables() const;
  const std::unordered_map<std::string, Label *> &getLabels() const;
  void addInstruction(Instruction *inst);
  const Label *getLabel(const std::string &name);
  void newBasicBlock();
  void newLinkedBasicBlock();
  const std::vector<BasicBlock *> &getBasicBlocks() const;
  std::string toStr() const;

private:
  std::string name;
  const Parameters *params{};
  std::vector<BasicBlock *> basicBlocks;
  std::unordered_map<std::string, const Variable *> variables;
  std::unordered_map<std::string, Label *> labels;
};

class Program {
public:
  Program();
  const std::vector<Function *> &getFunctions() const;
  void addFunction(Function *F);
  Function *getCurrFunction() const;
  void addInstruction(Instruction *inst);
  void newContext();
  void closeContext();
  const Variable *getVariable(const std::string &name) const;
  const Label *getLabel(const std::string &name) const;
  void newBasicBlock() const;
  void newLinkedBasicBlock() const;
  std::string toStr() const;

  Program(const Program &) = delete;
  Program &operator=(const Program &) = delete;

private:
  std::vector<Function *> functions;
  Context *currContext;
};

class Visitor {
public:
  virtual void visit(const Variable *var) = 0;
  virtual void visit(const Number *num) = 0;
  virtual void visit(const Arguments *args) = 0;
  virtual void visit(const Parameters *params) = 0;
  virtual void visit(const CompareOp *op) = 0;
  virtual void visit(const ArithOp *op) = 0;
  virtual void visit(const RuntimeFunction *func) = 0;
  virtual void visit(const FunctionName *name) = 0;
  virtual void visit(const Label *label) = 0;
  virtual void visit(const AssignInst *inst) = 0;
  virtual void visit(const ArithInst *inst) = 0;
  virtual void visit(const CompareInst *inst) = 0;
  virtual void visit(const LoadInst *inst) = 0;
  virtual void visit(const StoreInst *inst) = 0;
  virtual void visit(const RetInst *inst) = 0;
  virtual void visit(const RetValueInst *inst) = 0;
  virtual void visit(const LabelInst *inst) = 0;
  virtual void visit(const BranchInst *inst) = 0;
  virtual void visit(const CondBranchInst *inst) = 0;
  virtual void visit(const CallInst *inst) = 0;
  virtual void visit(const CallAssignInst *inst) = 0;
};

} // namespace L3
