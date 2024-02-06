#pragma once

#include <string>
using namespace std;

#include <L3.h>
#include <tile.h>

namespace L3 {
class VariableNode;

class TreeNode {
public:
  enum class Status { UNMATCHED, MATCHED, PARTIAL_MATCHED };

  const Tile *getMatchedTile() const;
  void setMatchedTile(const Tile *tile, bool edge);
  Status getStatus() const;
  virtual string toStr() const = 0;

private:
  const Tile *matchedTile;
  Status status;
};

class OperationNode : public TreeNode {
public:
  enum class Operation {
    ADD,
    SUB,
    MUL,
    AND,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    BR,
    COND_BR,
    CALL,
    RET,
    RET_VAL,
    STORE,
    LOAD,
    ASSIGN
  };

  OperationNode(Operation operation);
  Operation getOperation() const;
  VariableNode *getLeft();
  VariableNode *getRight();
  string toStr() const override;

private:
  const Operation operation;
  VariableNode *left, *right;
};

class VariableNode : public TreeNode {
public:
  VariableNode(const Variable *variable);
  const Variable *getVariable() const;
  OperationNode *getChild();
  string toStr() const override;

private:
  const Variable *variable;
  OperationNode *child;
};

const TreeNode &constructTree(Program *P);
} // namespace L3