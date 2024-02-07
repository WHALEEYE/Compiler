#pragma once
#include <unordered_set>
#include <vector>

#include <L3.h>
using namespace std;

namespace L3 {
class TreeNode;

class L2CodeBlockNode {
public:
  string toStr() const;

  void addInstructions(vector<string> &insts);
  void addChild(const L2CodeBlockNode *child);
  void removeChild(const L2CodeBlockNode *child);
  void setParent(const L2CodeBlockNode *parent);
  bool hasChild() const;

private:
  vector<string> insts;
  const L2CodeBlockNode *parent;
  unordered_set<const L2CodeBlockNode *> children;
};

class TilingResult {
public:
  TilingResult() = default;
  void addBlockRoot(const L2CodeBlockNode *root);
  string dump() const;

private:
  vector<const L2CodeBlockNode *> roots;

  TilingResult &operator=(const TilingResult &) = delete;
  TilingResult(const TilingResult &) = delete;
};

class Tile {
public:
  static const TilingResult &doTiling(vector<TreeNode *> roots);
  virtual int match(TreeNode *node) const = 0;
  virtual vector<TreeNode *> apply(TreeNode *node, TilingResult &result) const = 0;

private:
  static const std::unordered_set<const Tile *> tiles;
};

class ArithTile : public Tile {
public:
  int match(TreeNode *node) const override;
  vector<TreeNode *> apply(TreeNode *node, TilingResult &result) const override;
  static const ArithTile *getInstance();

private:
  ArithTile() = default;
  ArithTile(const ArithTile &) = delete;
  ArithTile &operator=(const ArithTile &) = delete;
  static const ArithTile *instance;
};

class CompareTile : public Tile {
public:
  int match(TreeNode *node) const override;
  vector<TreeNode *> apply(TreeNode *node, TilingResult &result) const override;
  static const CompareTile *getInstance();

private:
  CompareTile() = default;
  CompareTile(const CompareTile &) = delete;
  CompareTile &operator=(const CompareTile &) = delete;
  static const CompareTile *instance;
};

class AssignTile : public Tile {
public:
  int match(TreeNode *node) const override;
  vector<TreeNode *> apply(TreeNode *node, TilingResult &result) const override;
  static const AssignTile *getInstance();

private:
  AssignTile() = default;
  AssignTile(const AssignTile &) = delete;
  AssignTile &operator=(const AssignTile &) = delete;
  static const AssignTile *instance;
};

class StoreTile : public Tile {
public:
  int match(TreeNode *node) const override;
  vector<TreeNode *> apply(TreeNode *node, TilingResult &result) const override;
  static const StoreTile *getInstance();

private:
  StoreTile() = default;
  StoreTile(const StoreTile &) = delete;
  StoreTile &operator=(const StoreTile &) = delete;
  static const StoreTile *instance;
};

class LoadTile : public Tile {
public:
  int match(TreeNode *node) const override;
  vector<TreeNode *> apply(TreeNode *node, TilingResult &result) const override;
  static const LoadTile *getInstance();

private:
  LoadTile() = default;
  LoadTile(const LoadTile &) = delete;
  LoadTile &operator=(const LoadTile &) = delete;
  static const LoadTile *instance;
};

class CallTile : public Tile {
public:
  int match(TreeNode *node) const override;
  vector<TreeNode *> apply(TreeNode *node, TilingResult &result) const override;
  static const CallTile *getInstance();

private:
  CallTile() = default;
  CallTile(const CallTile &) = delete;
  CallTile &operator=(const CallTile &) = delete;
  static const CallTile *instance;
};

class CallAssignTile : public Tile {
public:
  int match(TreeNode *node) const override;
  vector<TreeNode *> apply(TreeNode *node, TilingResult &result) const override;
  static const CallAssignTile *getInstance();

private:
  CallAssignTile() = default;
  CallAssignTile(const CallAssignTile &) = delete;
  CallAssignTile &operator=(const CallAssignTile &) = delete;
  static const CallAssignTile *instance;
};

class ReturnTile : public Tile {
public:
  int match(TreeNode *node) const override;
  vector<TreeNode *> apply(TreeNode *node, TilingResult &result) const override;
  static const ReturnTile *getInstance();

private:
  ReturnTile() = default;
  ReturnTile(const ReturnTile &) = delete;
  ReturnTile &operator=(const ReturnTile &) = delete;
  static const ReturnTile *instance;
};

class ReturnValTile : public Tile {
public:
  int match(TreeNode *node) const override;
  vector<TreeNode *> apply(TreeNode *node, TilingResult &result) const override;
  static const ReturnValTile *getInstance();

private:
  ReturnValTile() = default;
  ReturnValTile(const ReturnValTile &) = delete;
  ReturnValTile &operator=(const ReturnValTile &) = delete;
  static const ReturnValTile *instance;
};

class BranchTile : public Tile {
public:
  int match(TreeNode *node) const override;
  vector<TreeNode *> apply(TreeNode *node, TilingResult &result) const override;
  static const BranchTile *getInstance();

private:
  BranchTile() = default;
  BranchTile(const BranchTile &) = delete;
  BranchTile &operator=(const BranchTile &) = delete;
  static const BranchTile *instance;
};

class CondBranchTile : public Tile {
public:
  int match(TreeNode *node) const override;
  vector<TreeNode *> apply(TreeNode *node, TilingResult &result) const override;
  static const CondBranchTile *getInstance();

private:
  CondBranchTile() = default;
  CondBranchTile(const CondBranchTile &) = delete;
  CondBranchTile &operator=(const CondBranchTile &) = delete;
  static const CondBranchTile *instance;
};

class LabelTile : public Tile {
public:
  int match(TreeNode *node) const override;
  vector<TreeNode *> apply(TreeNode *node, TilingResult &result) const override;
  static const LabelTile *getInstance();

private:
  LabelTile() = default;
  LabelTile(const LabelTile &) = delete;
  LabelTile &operator=(const LabelTile &) = delete;
  static const LabelTile *instance;
};

} // namespace L3