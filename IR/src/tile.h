#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

#include <L3.h>
#include <tree.h>

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
  unordered_set<const CodeBlock *> children;
};

class TilingResult {
public:
  TilingResult() = default;
  const vector<string> &assembleCode() const;

  TilingResult &operator=(const TilingResult &) = delete;
  TilingResult(const TilingResult &) = delete;

private:
  // Code block roots in order. Each root corresponds to a tree.
  vector<const CodeBlock *> roots;
  unordered_map<const TreeNode *, CodeBlock *> nodeToBlock;

  friend void addBlock(const TreeNode *root, const vector<const TreeNode *> &leaves, CodeBlock *newBlock,
                       TilingResult &result);
};

class Tile {
public:
  virtual int match(const TreeNode *node) const = 0;
  virtual vector<const TreeNode *> apply(const TreeNode *node, TilingResult &result) const = 0;
  static const unordered_set<const Tile *> &getTiles();

private:
  static const std::unordered_set<const Tile *> tiles;
};

class ArithTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, TilingResult &result) const override;
  static const ArithTile *getInstance();

  ArithTile(const ArithTile &) = delete;
  ArithTile &operator=(const ArithTile &) = delete;

private:
  ArithTile() = default;
  static const ArithTile *instance;
};

class CompareTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, TilingResult &result) const override;
  static const CompareTile *getInstance();

  CompareTile(const CompareTile &) = delete;
  CompareTile &operator=(const CompareTile &) = delete;

private:
  CompareTile() = default;
  static const CompareTile *instance;
};

class AssignTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, TilingResult &result) const override;
  static const AssignTile *getInstance();

  AssignTile(const AssignTile &) = delete;
  AssignTile &operator=(const AssignTile &) = delete;

private:
  AssignTile() = default;
  static const AssignTile *instance;
};

class StoreTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, TilingResult &result) const override;
  static const StoreTile *getInstance();

  StoreTile(const StoreTile &) = delete;
  StoreTile &operator=(const StoreTile &) = delete;

private:
  StoreTile() = default;
  static const StoreTile *instance;
};

class LoadTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, TilingResult &result) const override;
  static const LoadTile *getInstance();

  LoadTile(const LoadTile &) = delete;
  LoadTile &operator=(const LoadTile &) = delete;

private:
  LoadTile() = default;
  static const LoadTile *instance;
};

class CallTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, TilingResult &result) const override;
  static const CallTile *getInstance();

  CallTile(const CallTile &) = delete;
  CallTile &operator=(const CallTile &) = delete;

private:
  CallTile() = default;
  static const CallTile *instance;
};

class CallAssignTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, TilingResult &result) const override;
  static const CallAssignTile *getInstance();

  CallAssignTile(const CallAssignTile &) = delete;
  CallAssignTile &operator=(const CallAssignTile &) = delete;

private:
  CallAssignTile() = default;
  static const CallAssignTile *instance;
};

class ReturnTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, TilingResult &result) const override;
  static const ReturnTile *getInstance();

  ReturnTile(const ReturnTile &) = delete;
  ReturnTile &operator=(const ReturnTile &) = delete;

private:
  ReturnTile() = default;
  static const ReturnTile *instance;
};

class ReturnValTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, TilingResult &result) const override;
  static const ReturnValTile *getInstance();

  ReturnValTile(const ReturnValTile &) = delete;
  ReturnValTile &operator=(const ReturnValTile &) = delete;

private:
  ReturnValTile() = default;
  static const ReturnValTile *instance;
};

class BranchTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, TilingResult &result) const override;
  static const BranchTile *getInstance();

  BranchTile(const BranchTile &) = delete;
  BranchTile &operator=(const BranchTile &) = delete;

private:
  BranchTile() = default;
  static const BranchTile *instance;
};

class CondBranchTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, TilingResult &result) const override;
  static const CondBranchTile *getInstance();

  CondBranchTile(const CondBranchTile &) = delete;
  CondBranchTile &operator=(const CondBranchTile &) = delete;

private:
  CondBranchTile() = default;
  static const CondBranchTile *instance;
};

class LabelTile : public Tile {
public:
  int match(const TreeNode *node) const override;
  vector<const TreeNode *> apply(const TreeNode *node, TilingResult &result) const override;
  static const LabelTile *getInstance();

  LabelTile(const LabelTile &) = delete;
  LabelTile &operator=(const LabelTile &) = delete;

private:
  LabelTile() = default;
  static const LabelTile *instance;
};

const TilingResult &tileFunction(const Trees &trees);

} // namespace L3