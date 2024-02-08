#pragma once
#include "tree.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <L3.h>
using namespace std;

namespace L3 {
class TreeNode;

class CodeBlock {
public:
  string toStr() const;

  void addInstructions(vector<string> &insts);
  const vector<string> &getInstructions() const;
  void addChild(const CodeBlock *child);
  const unordered_set<const CodeBlock *> &getChildren() const;

private:
  vector<string> instructions;
  const CodeBlock *parent;
  unordered_set<const CodeBlock *> children;
};

class FunctionTilingResult {
public:
  FunctionTilingResult() = default;
  const vector<string> &assembleCode() const;

private:
  // Code block roots in order. Each root corresponds to a tree.
  vector<const CodeBlock *> roots;
  unordered_map<const TreeNode *, CodeBlock *> nodeToBlock;

  FunctionTilingResult &operator=(const FunctionTilingResult &) = delete;
  FunctionTilingResult(const FunctionTilingResult &) = delete;

  friend void addBlock(const TreeNode *root, const vector<const TreeNode *> &leaves,
                       CodeBlock *newBlock, FunctionTilingResult &result);
};

typedef unordered_map<const Function *, const FunctionTilingResult &> TilingResult;

class Tile {
public:
  virtual int match(const TreeNode *node) const = 0;
  virtual vector<const TreeNode *> apply(const TreeNode *node,
                                         FunctionTilingResult &result) const = 0;
  static const unordered_set<const Tile *> &getTiles();

private:
  static const std::unordered_set<const Tile *> tiles;
};

class ArithTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, FunctionTilingResult &result) const override;
  static const ArithTile *getInstance();

private:
  ArithTile() = default;
  ArithTile(const ArithTile &) = delete;
  ArithTile &operator=(const ArithTile &) = delete;
  static const ArithTile *instance;
};

class CompareTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, FunctionTilingResult &result) const override;
  static const CompareTile *getInstance();

private:
  CompareTile() = default;
  CompareTile(const CompareTile &) = delete;
  CompareTile &operator=(const CompareTile &) = delete;
  static const CompareTile *instance;
};

class AssignTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, FunctionTilingResult &result) const override;
  static const AssignTile *getInstance();

private:
  AssignTile() = default;
  AssignTile(const AssignTile &) = delete;
  AssignTile &operator=(const AssignTile &) = delete;
  static const AssignTile *instance;
};

class StoreTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, FunctionTilingResult &result) const override;
  static const StoreTile *getInstance();

private:
  StoreTile() = default;
  StoreTile(const StoreTile &) = delete;
  StoreTile &operator=(const StoreTile &) = delete;
  static const StoreTile *instance;
};

class LoadTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, FunctionTilingResult &result) const override;
  static const LoadTile *getInstance();

private:
  LoadTile() = default;
  LoadTile(const LoadTile &) = delete;
  LoadTile &operator=(const LoadTile &) = delete;
  static const LoadTile *instance;
};

class CallTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, FunctionTilingResult &result) const override;
  static const CallTile *getInstance();

private:
  CallTile() = default;
  CallTile(const CallTile &) = delete;
  CallTile &operator=(const CallTile &) = delete;
  static const CallTile *instance;
};

class CallAssignTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, FunctionTilingResult &result) const override;
  static const CallAssignTile *getInstance();

private:
  CallAssignTile() = default;
  CallAssignTile(const CallAssignTile &) = delete;
  CallAssignTile &operator=(const CallAssignTile &) = delete;
  static const CallAssignTile *instance;
};

class ReturnTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, FunctionTilingResult &result) const override;
  static const ReturnTile *getInstance();

private:
  ReturnTile() = default;
  ReturnTile(const ReturnTile &) = delete;
  ReturnTile &operator=(const ReturnTile &) = delete;
  static const ReturnTile *instance;
};

class ReturnValTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, FunctionTilingResult &result) const override;
  static const ReturnValTile *getInstance();

private:
  ReturnValTile() = default;
  ReturnValTile(const ReturnValTile &) = delete;
  ReturnValTile &operator=(const ReturnValTile &) = delete;
  static const ReturnValTile *instance;
};

class BranchTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, FunctionTilingResult &result) const override;
  static const BranchTile *getInstance();

private:
  BranchTile() = default;
  BranchTile(const BranchTile &) = delete;
  BranchTile &operator=(const BranchTile &) = delete;
  static const BranchTile *instance;
};

class CondBranchTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, FunctionTilingResult &result) const override;
  static const CondBranchTile *getInstance();

private:
  CondBranchTile() = default;
  CondBranchTile(const CondBranchTile &) = delete;
  CondBranchTile &operator=(const CondBranchTile &) = delete;
  static const CondBranchTile *instance;
};

class LabelTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, FunctionTilingResult &result) const override;
  static const LabelTile *getInstance();

private:
  LabelTile() = default;
  LabelTile(const LabelTile &) = delete;
  LabelTile &operator=(const LabelTile &) = delete;
  static const LabelTile *instance;
};

const TilingResult &doTiling(const TreeResult &treeResult);

} // namespace L3