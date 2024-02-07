#pragma once

#include <string>
using namespace std;

#include <L3.h>
#include <tile.h>

namespace L3 {
class OperandNode;

class TreeNode {
public:
  L2CodeBlockNode *getCodeBlock() const;
  void setCodeBlock(L2CodeBlockNode *codeBlock);
  virtual string toStr() const = 0;

private:
  L2CodeBlockNode *codeBlock;
};

class OperationNode : public TreeNode {};
class OperandNode : public TreeNode {
public:
  OperandNode(const Item *operand);
  const Item *getOperand() const;
  OperationNode *getChild();
  string toStr() const override;

private:
  const Item *operand;
  OperationNode *child;
};

class CallNode : public OperationNode {
public:
  void setCallee(OperandNode *callee);
  OperandNode *getCallee() const;
  void setArgs(OperandNode *args);
  OperandNode *getArgs() const;
  string toStr() const override;

private:
  OperandNode *callee, *args;
};

class ReturnNode : public TreeNode {
public:
  string toStr() const override;
};

class ReturnValNode : public OperationNode {
public:
  void setVal(OperandNode *val);
  OperandNode *getVal() const;
  string toStr() const override;

private:
  OperandNode *val;
};

class AssignNode : public TreeNode {
public:
  void setRhs(OperandNode *rhs);
  OperandNode *getRhs() const;
  string toStr() const override;

private:
  OperandNode *rhs;
};

class CompareNode : public OperationNode {
public:
  void setOp(const CompareOp *op);
  void setLhs(OperandNode *lhs);
  void setRhs(OperandNode *rhs);
  const CompareOp *getOp() const;
  OperandNode *getLhs() const;
  OperandNode *getRhs() const;
  string toStr() const override;

private:
  const CompareOp *op;
  OperandNode *lhs, *rhs;
};

class LoadNode : public OperationNode {
public:
  void setAddr(OperandNode *addr);
  OperandNode *getAddr() const;
  string toStr() const override;

private:
  OperandNode *addr;
};

class StoreNode : public OperationNode {
public:
  void setVal(OperandNode *val);
  OperandNode *getVal() const;
  string toStr() const override;

private:
  OperandNode *val;
};

class ArithmeticNode : public OperationNode {
public:
  void setOp(const ArithOp *op);
  void setLhs(OperandNode *lhs);
  void setRhs(OperandNode *rhs);
  const ArithOp *getOp() const;
  OperandNode *getLhs() const;
  OperandNode *getRhs() const;
  string toStr() const override;

private:
  const ArithOp *op;
  OperandNode *lhs, *rhs;
};

class BranchNode : public OperationNode {
public:
  void setLabel(OperandNode *label);
  OperandNode *getLabel() const;
  string toStr() const override;

private:
  OperandNode *label;
};

class CondBranchNode : public OperationNode {
public:
  void setCond(OperandNode *cond);
  void setLabel(OperandNode *label);
  OperandNode *getCond() const;
  OperandNode *getLabel() const;
  string toStr() const override;

private:
  OperandNode *cond, *label;
};

class LabelNode : public OperationNode {
public:
  void setLabel(const Label *label);
  const Label *getLabel() const;
  string toStr() const override;

private:
  const Label *label;
};

const vector<TreeNode *> &constructTrees(Function *F);
} // namespace L3