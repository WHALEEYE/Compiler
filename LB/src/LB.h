#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace LB {

class Visitor;
class Value;
class Function;

class Item {
public:
  virtual std::string toStr() const;
  virtual std::string getLAToken() const;
};

class VarType : public Item {
public:
  enum Type { INT64, ARRAY, TUPLE, CODE, VOID };
  virtual Type getType() const = 0;
};

class Int64Type : public VarType {
public:
  std::string toStr() const override;
  std::string getLAToken() const override;

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
  std::string getLAToken() const override;

private:
  int64_t dim;
};

class TupleType : public VarType {
public:
  std::string toStr() const override;
  std::string getLAToken() const override;

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
  std::string getLAToken() const override;

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
  std::string getLAToken() const override;

  Type getType() const override;

  VoidType(const VoidType &) = delete;
  VoidType &operator=(const VoidType &) = delete;
  static VoidType *getInstance();

private:
  VoidType() = default;
  static VoidType *instance;
};

class Value : public Item {};

class Name : public Value {};

class Variable : public Name {
public:
  std::string rawName, globName;
  VarType *varType{};

  Variable(const std::string &rawName, const std::string &globName, VarType *varType);

  std::string toStr() const override;
  std::string getLAToken() const override;
};

class VariableList : public Item {
public:
  std::vector<Variable *> variables;

  std::string toStr() const override;
  std::string getLAToken() const override;
};

class Number : public Value {
public:
  explicit Number(int64_t val);
  int64_t getValue() const;
  void setValue(int64_t val);

  std::string toStr() const override;
  std::string getLAToken() const override;

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
  std::string getLAToken() const override;

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
  std::string getLAToken() const override;

private:
  std::vector<const Value *> args;
};

class Parameters : public Item {
public:
  Parameters() = default;
  void addParamToHead(const Variable *param);
  void addParamToTail(const Variable *param);
  const std::vector<const Variable *> &getParams() const;

  std::string toStr() const override;
  std::string getLAToken() const override;

private:
  std::vector<const Variable *> params;
};

class CmpOp : public Item {
public:
  enum ID { LESS_THAN, LESS_EQUAL, EQUAL, GREATER_EQUAL, GREATER_THAN };
  static CmpOp *getOp(ID id);

  std::string getName() const;
  ID getID() const;

  std::string toStr() const override;
  std::string getLAToken() const override;

private:
  CmpOp(ID id, std::string name);
  static const std::unordered_map<ID, CmpOp *> enumMap;

  const ID id;
  std::string name;
};

class Op : public Item {
public:
  enum ID { ADD, SUB, MUL, AND, LS, RS };
  static Op *getOp(ID id);

  std::string getName() const;
  ID getID() const;

  std::string toStr() const override;
  std::string getLAToken() const override;

private:
  Op(ID id, std::string name);
  static const std::unordered_map<ID, Op *> enumMap;

  const ID id;
  std::string name;
};

class RuntimeFunction : public Name {
public:
  enum ID { PRINT, INPUT };
  static RuntimeFunction *getRuntimeFunction(ID id);

  std::string getName() const;

  std::string toStr() const override;
  std::string getLAToken() const override;

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
  std::string getLAToken() const override;

private:
  std::string name;
};

class Label : public Item {
public:
  explicit Label(std::string name);
  std::string getName() const;

  std::string toStr() const override;
  std::string getLAToken() const override;

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

/*
 * Instructions.
 */
class DeclarationInst : public Instruction {
public:
  VariableList *declaredVars;

  DeclarationInst(VariableList *declaredVars);
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
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

class CmpInst : public Instruction {
public:
  Variable *rst;
  CmpOp *op;
  Value *lhs, *rhs;

  CmpInst(Variable *rst, Value *lhs, CmpOp *op, Value *rhs);

  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
};

class OpInst : public Instruction {
public:
  OpInst(const Variable *rst, const Value *lhs, const Op *op, const Value *rhs);
  const Variable *getRst() const;
  const Value *getLhs() const;
  const Op *getOp() const;
  const Value *getRhs() const;
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;

private:
  const Variable *rst;
  const Value *lhs;
  const Op *op;
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

class IfInst : public Instruction {
public:
  Label *trueLabel, *falseLabel;
  Value *lhs, *rhs;
  CmpOp *op;

  IfInst(Value *lhs, CmpOp *op, Value *rhs, Label *trueLabel, Label *falseLabel);
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
};

class WhileInst : public Instruction {
public:
  Label *bodyLabel, *exitLabel;
  Value *lhs, *rhs;
  CmpOp *op;

  WhileInst(Value *lhs, CmpOp *op, Value *rhs, Label *bodyLabel, Label *exitLabel);
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
};

class ContinueInst : public Instruction {
public:
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
};

class BreakInst : public Instruction {
public:
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
};

class RetInst : public Instruction {
public:
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
};

class RetValueInst : public Instruction {
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

class GotoInst : public Instruction {
public:
  Label *label;

  explicit GotoInst(Label *label);
  std::string toStr() const override;
  void accept(Visitor &visitor) const override;
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

class Scope {
public:
  explicit Scope(Scope *parent);
  Scope *getParent() const;
  Variable *declareVariable(const std::string &rawName, const std::string &globName, VarType *type);
  Variable *getVariable(const std::string &rawName);

  void setType(VarType *type);

private:
  Scope *parent;
  std::unordered_map<std::string, Variable *> variables;
  VarType *currType;
};

class Function {
public:
  explicit Function();

  std::vector<Instruction *> &getInstructions();

  std::string getName() const;
  void setName(const std::string &name);

  const VarType *getReturnType() const;
  void setReturnType(const VarType *type);

  const Parameters *getParams() const;
  void addParam(const std::string &name, VarType *type);

  Variable *declareVariable(const std::string &rawName);
  Variable *getVariable(const std::string &rawName);
  std::string getNewGlobVarName();

  Label *getLabel(const std::string &name);
  const Label *generateNewLabel();
  const std::unordered_map<std::string, Label *> &getLabels() const;
  std::string generateNewLabelName();

  void addInstruction(Instruction *inst, int64_t lineno);

  void enterScope();
  void exitScope();
  void setCurrType(VarType *type);

  std::string toStr() const;

private:
  std::string name;
  const VarType *returnType{};
  Parameters *params{};
  std::vector<Instruction *> instructions;

  Scope *currScope{};
  VarType *currType{};

  // the name of a label may need to be changed later
  std::unordered_map<std::string, Label *> labels;

  std::string varPrefix;
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

  std::string toStr() const;

  Program(const Program &) = delete;
  Program &operator=(const Program &) = delete;

private:
  std::vector<Function *> functions;
};

class Visitor {
public:
  virtual void visit(const DeclarationInst *inst) = 0;
  virtual void visit(const AssignInst *inst) = 0;
  virtual void visit(const CmpInst *inst) = 0;
  virtual void visit(const OpInst *inst) = 0;
  virtual void visit(const LoadInst *inst) = 0;
  virtual void visit(const StoreInst *inst) = 0;
  virtual void visit(const ArrayLenInst *inst) = 0;
  virtual void visit(const TupleLenInst *inst) = 0;
  virtual void visit(const NewArrayInst *inst) = 0;
  virtual void visit(const NewTupleInst *inst) = 0;
  virtual void visit(const IfInst *inst) = 0;
  virtual void visit(const WhileInst *inst) = 0;
  virtual void visit(const ContinueInst *inst) = 0;
  virtual void visit(const BreakInst *inst) = 0;
  virtual void visit(const RetInst *inst) = 0;
  virtual void visit(const RetValueInst *inst) = 0;
  virtual void visit(const LabelInst *inst) = 0;
  virtual void visit(const GotoInst *inst) = 0;
  virtual void visit(const CallInst *inst) = 0;
  virtual void visit(const CallAssignInst *inst) = 0;
};

} // namespace LB
