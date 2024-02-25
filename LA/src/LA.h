#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace LA {

class Visitor;
class Value;
class Function;

class Item {
public:
  virtual std::string toStr() const;
};

class VarType : public Item {
public:
  enum Type { INT64, ARRAY, TUPLE, CODE, VOID };
  virtual Type getType() const = 0;
};

class Int64Type : public VarType {
public:
  std::string toStr() const override;

  Type getType() const override;

  Int64Type(const Int64Type &) = delete;
  Int64Type &operator=(const Int64Type &) = delete;
  static Int64Type *getInstance();

private:
  Int64Type() = default;
  static Int64Type *instance;
};

class ArrayType : public VarType {
public:
  void increaseDim();
  int64_t getDim() const;
  Type getType() const override;

  std::string toStr() const override;

private:
  int64_t dim;
};

class TupleType : public VarType {
public:
  std::string toStr() const override;

  Type getType() const override;

  TupleType(const TupleType &) = delete;
  TupleType &operator=(const TupleType &) = delete;
  static TupleType *getInstance();

private:
  TupleType() = default;
  static TupleType *instance;
};

class CodeType : public VarType {
public:
  std::string toStr() const override;

  Type getType() const override;

  CodeType(const CodeType &) = delete;
  CodeType &operator=(const CodeType &) = delete;

  static CodeType *getInstance();

private:
  CodeType() = default;
  static CodeType *instance;
};

class VoidType : public VarType {
public:
  std::string toStr() const override;

  Type getType() const override;

  VoidType(const VoidType &) = delete;
  VoidType &operator=(const VoidType &) = delete;
  static VoidType *getInstance();

private:
  VoidType() = default;
  static VoidType *instance;
};

class Value : public Item {};

class Name : public Value {
public:
  virtual std::string getPrefixedName() const = 0;
};

class Variable : public Name {
public:
  Variable(const std::string &name, const VarType *varType);
  const std::string &getName() const;
  const VarType *getVarType() const;
  std::string toStr() const override;

  std::string getPrefixedName() const override;
  std::string getDeclStr() const;

private:
  const VarType *varType{};
  const std::string name;
};

class Number : public Value {
public:
  explicit Number(int64_t val);
  int64_t getValue() const;
  void setValue(int64_t val);
  std::string toStr() const override;

private:
  int64_t value;
};

class MemoryLocation : public Item {
public:
  MemoryLocation(const Variable *base);
  const Variable *getBase() const;
  void addIndex(const Value *index);
  const std::vector<const Value *> &getIndices() const;

  std::string toStr() const override;

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

private:
  std::vector<const Value *> args;
};

class Parameters : public Item {
public:
  Parameters() = default;
  void addParamToHead(const Variable *param);
  const std::vector<const Variable *> &getParams() const;
  std::string toStr() const override;

private:
  std::vector<const Variable *> params;
};

class Operator : public Item {
public:
  enum ID { ADD, SUB, MUL, AND, LS, RS, LESS_THAN, LESS_EQUAL, EQUAL, GREATER_EQUAL, GREATER_THAN };
  static Operator *getOperator(ID id);

  std::string getName() const;
  ID getID() const;
  std::string toStr() const override;

private:
  Operator(ID id, std::string name);
  static const std::unordered_map<ID, Operator *> enumMap;

  const ID id;
  std::string name;
};

class RuntimeFunction : public Name {
public:
  enum ID { PRINT, INPUT };
  static RuntimeFunction *getRuntimeFunction(ID id);

  std::string getName() const;
  std::string toStr() const override;

  std::string getPrefixedName() const override;

private:
  explicit RuntimeFunction(std::string name);
  static const std::unordered_map<ID, RuntimeFunction *> enumMap;

  std::string name;
};

class UserFunction : public Name {
public:
  explicit UserFunction(std::string name);
  std::string getName() const;
  std::string toStr() const override;

  std::string getPrefixedName() const override;

private:
  std::string name;
};

class Label : public Item {
public:
  explicit Label(std::string name);
  std::string getName() const;
  std::string toStr() const override;

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
  int64_t lineno;
};

class TerminatorInst : public Instruction {};

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
  AssignInst(const Variable *lval, const Value *rval);
  const Variable *getLhs() const;
  const Value *getRhs() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *lhs;
  const Value *rhs;
};

class OpInst : public Instruction {
public:
  OpInst(const Variable *rst, const Value *lhs, const Operator *op, const Value *rhs);
  const Variable *getRst() const;
  const Value *getLhs() const;
  const Operator *getOp() const;
  const Value *getRhs() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *rst;
  const Value *lhs;
  const Operator *op;
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
  const Variable *getArray() const;
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
  const Variable *getTuple() const;

  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *result;
  const Variable *base;
};

class NewArrayInst : public Instruction {
public:
  NewArrayInst(const Variable *array, const std::vector<const Value *> &sizes);
  const Variable *getArray() const;
  const std::vector<const Value *> &getSizes() const;

  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *array;
  const std::vector<const Value *> sizes;
};

class NewTupleInst : public Instruction {
public:
  NewTupleInst(const Variable *tuple, const Value *size);

  const Variable *getTuple() const;
  const Value *getSize() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *tuple;
  const Value *size;
};

class RetInst : public TerminatorInst {
public:
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
};

class RetValueInst : public TerminatorInst {
public:
  explicit RetValueInst(const Value *value);
  const Value *getValue() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Value *value;
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

class BranchInst : public TerminatorInst {
public:
  explicit BranchInst(const Label *label);
  const Label *getLabel() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Label *label;
};

class CondBranchInst : public TerminatorInst {
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
  CallInst(const Name *callee, const Arguments *args);
  const Name *getCallee() const;
  const Arguments *getArgs() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Name *callee;
  const Arguments *args;
};

class CallAssignInst : public Instruction {
public:
  CallAssignInst(const Variable *rst, const Name *callee, const Arguments *args);
  const Variable *getRst() const;
  const Name *getCallee() const;
  const Arguments *getArgs() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *rst;
  const Name *callee;
  const Arguments *args;
};

/*
 * Structures.
 */
class BasicBlock {
public:
  BasicBlock();
  const std::vector<const Instruction *> &getInstructions() const;
  void addInstruction(const Instruction *inst);
  const std::unordered_set<BasicBlock *> &getPredecessors() const;
  void addPredecessor(BasicBlock *BB);
  void removePredecessor(BasicBlock *BB);
  const std::unordered_set<BasicBlock *> &getSuccessors() const;
  void addSuccessor(BasicBlock *BB);
  void removeSuccessor(BasicBlock *BB);
  bool empty() const;
  std::string toStr() const;

  const Label *getLabel() const;
  void setLabel(const Label *label);

  const TerminatorInst *getTerminator() const;
  void setTerminator(const TerminatorInst *terminator);

  friend void initializeVariables(Function *F);

private:
  const Label *label;
  std::vector<const Instruction *> instructions;
  const TerminatorInst *terminator;

  std::unordered_set<BasicBlock *> predecessors;
  std::unordered_set<BasicBlock *> successors;
};

class Program;

class Function {
public:
  explicit Function();

  std::string getName() const;
  void setName(const std::string &name);

  const VarType *getReturnType() const;
  void setReturnType(const VarType *type);

  const Parameters *getParams() const;
  void setParams(const Parameters *parameters);

  void defineVariable(const std::string &name, const VarType *type);
  Variable *getVariable(const std::string &name);
  const std::unordered_map<std::string, Variable *> &getVariables() const;
  bool hasVariable(const std::string &name) const;
  std::string generateNewVariableName();

  Label *getLabel(const std::string &name);
  const Label *generateNewLabel();
  const std::unordered_map<std::string, Label *> &getLabels() const;
  std::string generateNewLabelName();

  void addInstruction(Instruction *inst);

  BasicBlock *newBasicBlock();
  BasicBlock *getCurrBasicBlock() const;
  const std::vector<BasicBlock *> &getBasicBlocks() const;

  std::string toStr() const;

  friend void formatBasicBlock(Function *F);

private:
  std::string name;
  const VarType *returnType{};
  const Parameters *params{};
  std::vector<BasicBlock *> basicBlocks;
  std::unordered_map<std::string, Variable *> variables;
  // the name of a label may need to be changed later
  std::unordered_map<std::string, Label *> labels;

  std::string longestVarName;
  int64_t varCounter;
  std::string longestLabelName;
  int64_t labelCounter;
};

class Program {
public:
  Program();
  const std::vector<Function *> &getFunctions() const;
  void addFunction(Function *F);
  Function *getCurrFunction() const;

  // easy access to the current function
  void addInstructionToCurrFunc(Instruction *inst, int64_t lineno);
  void declareVariableInCurrFunc(const std::string &name, const VarType *type);
  Variable *getVariableInCurrFunc(const std::string &name) const;
  bool currFuncHasVariable(const std::string &name) const;
  Label *getLabelInCurrFunc(const std::string &name);
  BasicBlock *newBasicBlock() const;

  std::string toStr() const;

  std::string getNewGlobalSymbol();

  Program(const Program &) = delete;
  Program &operator=(const Program &) = delete;

private:
  std::vector<Function *> functions;
};

class Visitor {
public:
  virtual void visit(const DeclarationInst *inst) = 0;
  virtual void visit(const AssignInst *inst) = 0;
  virtual void visit(const OpInst *inst) = 0;
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

} // namespace LA
