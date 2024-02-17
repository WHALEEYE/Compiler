#pragma once

#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

#include <L3.h>
#include <liveness_analyzer.h>

namespace L3 {

class OperandNode;

class TreeNode {
public:
  virtual string toStr() const = 0;
};

class OperationNode : public TreeNode {};
class OperandNode : public TreeNode {
public:
  enum Status { MERGABLE, UNMERGABLE, MERGED };
  explicit OperandNode(const Item *operand);
  const Item *getOperand() const;
  const OperationNode *getChild() const;
  void setChild(OperationNode *child);
  string toStr() const override;

  void setStatus(Status status) { this->status = status; }
  Status getStatus() const { return status; }

private:
  const Item *operand;
  const OperationNode *child;
  Status status{};
};

class CallNode : public OperationNode {
public:
  void setCallee(const OperandNode *callee);
  const OperandNode *getCallee() const;
  void setArgs(const OperandNode *args);
  const OperandNode *getArgs() const;
  string toStr() const override;

private:
  const OperandNode *callee{}, *args{};
};

class ReturnNode : public OperationNode {
public:
  string toStr() const override;
};

class ReturnValNode : public OperationNode {
public:
  void setVal(const OperandNode *val);
  const OperandNode *getVal() const;
  string toStr() const override;

private:
  const OperandNode *val{};
};

class AssignNode : public OperationNode {
public:
  void setRhs(const OperandNode *rhs);
  const OperandNode *getRhs() const;
  string toStr() const override;

private:
  const OperandNode *rhs{};
};

class CompareNode : public OperationNode {
public:
  void setOp(const CompareOp *op);
  void setLhs(const OperandNode *lhs);
  void setRhs(const OperandNode *rhs);
  const CompareOp *getOp() const;
  const OperandNode *getLhs() const;
  const OperandNode *getRhs() const;
  string toStr() const override;

private:
  const CompareOp *op{};
  const OperandNode *lhs{}, *rhs{};
};

class LoadNode : public OperationNode {
public:
  void setAddr(const OperandNode *addr);
  const OperandNode *getAddr() const;
  string toStr() const override;

private:
  const OperandNode *addr{};
};

class StoreNode : public OperationNode {
public:
  void setVal(const OperandNode *val);
  void setAddr(const OperandNode *addr);
  const OperandNode *getVal() const;
        const OperandNode *getAddr() const;
  string toStr() const override;

private:
  const OperandNode *val{};
  const OperandNode *addr{};
};

class ArithmeticNode : public OperationNode {
public:
  void setOp(const ArithOp *op);
  void setLhs(const OperandNode *lhs);
  void setRhs(const OperandNode *rhs);
  const ArithOp *getOp() const;
  const OperandNode *getLhs() const;
  const OperandNode *getRhs() const;
  string toStr() const override;

private:
  const ArithOp *op{};
  const OperandNode *lhs{}, *rhs{};
};

class BranchNode : public OperationNode {
public:
  void setLabel(const OperandNode *label);
  const OperandNode *getLabel() const;
  string toStr() const override;

private:
  const OperandNode *label{};
};

class CondBranchNode : public OperationNode {
public:
  void setCond(const OperandNode *cond);
  void setLabel(const OperandNode *label);
  const OperandNode *getCond() const;
  const OperandNode *getLabel() const;
  string toStr() const override;

private:
  const OperandNode *cond{}, *label{};
};

class LabelNode : public OperationNode {
public:
  void setLabel(const Label *label);
  const Label *getLabel() const;
  string toStr() const override;

private:
  const Label *label{};
};

typedef vector<const TreeNode *> Trees;

const Trees &constructTrees(const Function *F, const LivenessResult &liveness);

} // namespace L3