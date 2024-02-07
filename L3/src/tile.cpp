#include <stdexcept>

#include <L3.h>
#include <code_generator.h>
#include <tile.h>
#include <tree.h>
#include <unordered_set>
#include <vector>

namespace L3 {

const TilingResult &Tile::doTiling(vector<TreeNode *> roots) {
  auto result = new TilingResult();

  int maxCost = 0;
  const Tile *maxTile = nullptr;
  vector<TreeNode *> subRoots = roots;
  TreeNode *curr;
  while (!subRoots.empty()) {
    curr = subRoots.back();
    subRoots.pop_back();

    for (auto tile : tiles) {
      int cost = tile->match(curr);
      if (cost > maxCost) {
        maxCost = cost;
        maxTile = tile;
      }
    }

    if (maxCost == 0)
      throw runtime_error("No tile matched");

    auto edgeNodes = maxTile->apply(curr, *result);
    for (auto node : edgeNodes) {
      if (!dynamic_cast<OperandNode *>(node))
        throw runtime_error("Invalid edge node");
      if (dynamic_cast<OperandNode *>(node)->getChild())
        subRoots.push_back(node);
    }
  }

  return *result;
}

string L2CodeBlockNode::toStr() const {
  string ret;
  for (auto inst : insts)
    ret += inst + "\n";
  return ret;
}

void L2CodeBlockNode::addInstructions(vector<string> &insts) {
  this->insts.insert(this->insts.end(), insts.begin(), insts.end());
}
void L2CodeBlockNode::addChild(const L2CodeBlockNode *child) { children.insert(child); }
void L2CodeBlockNode::removeChild(const L2CodeBlockNode *child) { children.erase(child); }
void L2CodeBlockNode::setParent(const L2CodeBlockNode *parent) { this->parent = parent; }
bool L2CodeBlockNode::hasChild() const { return !children.empty(); }

const unordered_set<const Tile *> Tile::tiles = {ArithTile::getInstance()};

int ArithTile::match(TreeNode *node) const {
  if (!dynamic_cast<OperandNode *>(node))
    return 0;
  auto rst = dynamic_cast<OperandNode *>(node);
  if (!dynamic_cast<const Variable *>(rst->getOperand()) || !rst->getChild())
    return 0;
  if (!dynamic_cast<ArithmeticNode *>(rst->getChild()))
    return 0;

  return 3;
}

vector<TreeNode *> ArithTile::apply(TreeNode *node, TilingResult &result) const {
  auto rst = dynamic_cast<OperandNode *>(node);
  auto op = dynamic_cast<ArithmeticNode *>(rst->getChild());
  auto lhs = op->getLhs(), rhs = op->getRhs();
  vector<TreeNode *> edge = {lhs, rhs};

  L2CodeBlockNode *block = new L2CodeBlockNode();

  block->setParent(node->getCodeBlock());
  if (!node->getCodeBlock())
    result.addBlockRoot(block);
  else
    node->getCodeBlock()->addChild(block);

  for (auto eNode : edge)
    eNode->setCodeBlock(block);

  auto insts = generateArithmetic(rst->toStr(), lhs->toStr(), op->toStr(), rhs->toStr());
  block->addInstructions(insts);
  return edge;
}
const ArithTile *ArithTile::getInstance() {
  if (!instance)
    instance = new ArithTile();
  return instance;
}
const ArithTile *ArithTile::instance = nullptr;

int CompareTile::match(TreeNode *node) const {
  if (!dynamic_cast<OperandNode *>(node))
    return 0;
  auto rst = dynamic_cast<OperandNode *>(node);
  if (!dynamic_cast<const Variable *>(rst->getOperand()) || !rst->getChild())
    return 0;
  if (!dynamic_cast<CompareNode *>(rst->getChild()))
    return 0;

  return 3;
}
vector<TreeNode *> CompareTile::apply(TreeNode *node, TilingResult &result) const {
  auto rst = dynamic_cast<OperandNode *>(node);
  auto op = dynamic_cast<CompareNode *>(rst->getChild());
  auto lhs = op->getLhs(), rhs = op->getRhs();
  vector<TreeNode *> edge = {lhs, rhs};
  string op_str, lhs_str, rhs_str, rst_str = rst->toStr();
  switch (op->getOp()->getID()) {
  case CompareOp::ID::EQUAL:
  case CompareOp::ID::LESS_EQUAL:
  case CompareOp::ID::LESS_THAN:
    op_str = op->toStr();
    lhs_str = lhs->toStr();
    rhs_str = rhs->toStr();
    break;
  case CompareOp::ID::GREATER_EQUAL:
    op_str = CompareOp::getCompareOp(CompareOp::ID::LESS_EQUAL)->toStr();
    lhs_str = rhs->toStr();
    rhs_str = lhs->toStr();
    break;
  case CompareOp::ID::GREATER_THAN:
    op_str = CompareOp::getCompareOp(CompareOp::ID::LESS_THAN)->toStr();
    lhs_str = rhs->toStr();
    rhs_str = lhs->toStr();
    break;
  default:
    throw runtime_error("Invalid compare op");
  }

  L2CodeBlockNode *block = new L2CodeBlockNode();

  block->setParent(node->getCodeBlock());
  if (!node->getCodeBlock())
    result.addBlockRoot(block);
  else
    node->getCodeBlock()->addChild(block);

  for (auto eNode : edge)
    eNode->setCodeBlock(block);

  auto insts = generateCompare(rst_str, lhs_str, op_str, rhs_str);
  block->addInstructions(insts);
  return edge;
}
const CompareTile *CompareTile::getInstance() {
  if (!instance)
    instance = new CompareTile();
  return instance;
}
const CompareTile *CompareTile::instance = nullptr;

int StoreTile::match(TreeNode *node) const {
  if (!dynamic_cast<OperandNode *>(node))
    return 0;
  auto addr = dynamic_cast<OperandNode *>(node);
  if (!dynamic_cast<const Variable *>(addr->getOperand()) || !addr->getChild())
    return 0;
  if (!dynamic_cast<StoreNode *>(addr->getChild()))
    return 0;
  return 2;
}
vector<TreeNode *> StoreTile::apply(TreeNode *node, TilingResult &result) const {
  auto addr = dynamic_cast<OperandNode *>(node);
  auto op = dynamic_cast<StoreNode *>(addr->getChild());
  auto val = op->getVal();
  vector<TreeNode *> edge = {val};

  L2CodeBlockNode *block = new L2CodeBlockNode();

  block->setParent(node->getCodeBlock());
  if (!node->getCodeBlock())
    result.addBlockRoot(block);
  else
    node->getCodeBlock()->addChild(block);

  for (auto eNode : edge)
    eNode->setCodeBlock(block);

  auto insts = generateStore(addr->toStr(), val->toStr());
  block->addInstructions(insts);
  return edge;
}
const StoreTile *StoreTile::getInstance() {
  if (!instance)
    instance = new StoreTile();
  return instance;
}
const StoreTile *StoreTile::instance = nullptr;

int LoadTile::match(TreeNode *node) const {
  if (!dynamic_cast<OperandNode *>(node))
    return 0;
  auto lvalNode = dynamic_cast<OperandNode *>(node);
  if (!dynamic_cast<const Variable *>(lvalNode->getOperand()) || !lvalNode->getChild())
    return 0;
  if (!dynamic_cast<LoadNode *>(lvalNode->getChild()))
    return 0;
  return 2;
}
vector<TreeNode *> LoadTile::apply(TreeNode *node, TilingResult &result) const {
  auto val = dynamic_cast<OperandNode *>(node);
  auto op = dynamic_cast<LoadNode *>(val->getChild());
  auto addr = op->getAddr();
  vector<TreeNode *> edge = {addr};

  L2CodeBlockNode *block = new L2CodeBlockNode();

  block->setParent(node->getCodeBlock());
  if (!node->getCodeBlock())
    result.addBlockRoot(block);
  else
    node->getCodeBlock()->addChild(block);

  for (auto eNode : edge)
    eNode->setCodeBlock(block);

  auto insts = generateLoad(val->toStr(), addr->toStr());
  block->addInstructions(insts);
  return edge;
}
const LoadTile *LoadTile::getInstance() {
  if (!instance)
    instance = new LoadTile();
  return instance;
}
const LoadTile *LoadTile::instance = nullptr;

int AssignTile::match(TreeNode *node) const {
  if (!dynamic_cast<OperandNode *>(node))
    return 0;
  auto lvalNode = dynamic_cast<OperandNode *>(node);
  if (!dynamic_cast<const Variable *>(lvalNode->getOperand()) || !lvalNode->getChild())
    return 0;
  if (!dynamic_cast<AssignNode *>(lvalNode->getChild()))
    return 0;
  return 2;
}
vector<TreeNode *> AssignTile::apply(TreeNode *node, TilingResult &result) const {
  auto lhs = dynamic_cast<OperandNode *>(node);
  auto op = dynamic_cast<AssignNode *>(lhs->getChild());
  auto rhs = op->getRhs();
  vector<TreeNode *> edge = {lhs};

  L2CodeBlockNode *block = new L2CodeBlockNode();

  block->setParent(node->getCodeBlock());
  if (!node->getCodeBlock())
    result.addBlockRoot(block);
  else
    node->getCodeBlock()->addChild(block);

  for (auto eNode : edge)
    eNode->setCodeBlock(block);

  auto insts = generateAssign(lhs->toStr(), rhs->toStr());
  block->addInstructions(insts);
  return edge;
}
const AssignTile *AssignTile::getInstance() {
  if (!instance)
    instance = new AssignTile();
  return instance;
}
const AssignTile *AssignTile::instance = nullptr;

int BranchTile::match(TreeNode *node) const {
  if (!dynamic_cast<BranchNode *>(node))
    return 0;
  return 1;
}
vector<TreeNode *> BranchTile::apply(TreeNode *node, TilingResult &result) const {
  auto op = dynamic_cast<BranchNode *>(node);
  auto label = op->getLabel();
  vector<TreeNode *> edge = {label};

  L2CodeBlockNode *block = new L2CodeBlockNode();

  block->setParent(node->getCodeBlock());
  if (!node->getCodeBlock())
    result.addBlockRoot(block);
  else
    node->getCodeBlock()->addChild(block);

  for (auto eNode : edge)
    eNode->setCodeBlock(block);

  auto insts = generateBranch(label->toStr());
  block->addInstructions(insts);
  return edge;
}
const BranchTile *BranchTile::getInstance() {
  if (!instance)
    instance = new BranchTile();
  return instance;
}
const BranchTile *BranchTile::instance = nullptr;

int CondBranchTile::match(TreeNode *node) const {
  if (!dynamic_cast<CondBranchNode *>(node))
    return 0;
  return 2;
}
vector<TreeNode *> CondBranchTile::apply(TreeNode *node, TilingResult &result) const {
  auto op = dynamic_cast<CondBranchNode *>(node);
  auto cond = op->getCond(), label = op->getLabel();
  vector<TreeNode *> edge = {cond};

  L2CodeBlockNode *block = new L2CodeBlockNode();

  block->setParent(node->getCodeBlock());
  if (!node->getCodeBlock())
    result.addBlockRoot(block);
  else
    node->getCodeBlock()->addChild(block);

  for (auto eNode : edge)
    eNode->setCodeBlock(block);

  auto insts = generateCondBranch(cond->toStr(), label->toStr());
  block->addInstructions(insts);
  return edge;
}
const CondBranchTile *CondBranchTile::getInstance() {
  if (!instance)
    instance = new CondBranchTile();
  return instance;
}
const CondBranchTile *CondBranchTile::instance = nullptr;

int ReturnTile::match(TreeNode *node) const {
  if (!dynamic_cast<ReturnNode *>(node))
    return 0;
  return 1;
}
vector<TreeNode *> ReturnTile::apply(TreeNode *node, TilingResult &result) const {
  L2CodeBlockNode *block = new L2CodeBlockNode();

  block->setParent(node->getCodeBlock());
  if (!node->getCodeBlock())
    result.addBlockRoot(block);
  else
    node->getCodeBlock()->addChild(block);

  auto insts = generateReturn();
  block->addInstructions(insts);
  return {};
}
const ReturnTile *ReturnTile::getInstance() {
  if (!instance)
    instance = new ReturnTile();
  return instance;
}
const ReturnTile *ReturnTile::instance = nullptr;

int ReturnValTile::match(TreeNode *node) const {
  if (!dynamic_cast<ReturnValNode *>(node))
    return 0;
  return 1;
}
vector<TreeNode *> ReturnValTile::apply(TreeNode *node, TilingResult &result) const {
  auto op = dynamic_cast<ReturnValNode *>(node);
  auto val = op->getVal();
  vector<TreeNode *> edge = {val};

  L2CodeBlockNode *block = new L2CodeBlockNode();

  block->setParent(node->getCodeBlock());
  if (!node->getCodeBlock())
    result.addBlockRoot(block);
  else
    node->getCodeBlock()->addChild(block);

  for (auto eNode : edge)
    eNode->setCodeBlock(block);
  auto insts = generateReturnVal(val->toStr());
  block->addInstructions(insts);
  return edge;
}
const ReturnValTile *ReturnValTile::getInstance() {
  if (!instance)
    instance = new ReturnValTile();
  return instance;
}
const ReturnValTile *ReturnValTile::instance = nullptr;

int CallTile::match(TreeNode *node) const {
  if (!dynamic_cast<CallNode *>(node))
    return 0;
  return 2;
}
vector<TreeNode *> CallTile::apply(TreeNode *node, TilingResult &result) const {
  auto op = dynamic_cast<CallNode *>(node);
  auto callee = op->getCallee();
  auto args = op->getArgs();

  // call tree will always be independent bcs it's out of context
  L2CodeBlockNode *block = new L2CodeBlockNode();
  result.addBlockRoot(block);
  vector<string> argStrs;
  auto arguments = dynamic_cast<const Arguments *>(args->getOperand());
  for (auto arg : arguments->getArgs())
    argStrs.push_back(arg->toStr());

  auto insts = generateCall(callee->toStr(), argStrs);
  block->addInstructions(insts);
  return {};
}
const CallTile *CallTile::getInstance() {
  if (!instance)
    instance = new CallTile();
  return instance;
}
const CallTile *CallTile::instance = nullptr;

int CallAssignTile::match(TreeNode *node) const {
  if (!dynamic_cast<OperandNode *>(node))
    return 0;
  auto rst = dynamic_cast<OperandNode *>(node);
  if (!dynamic_cast<const Variable *>(rst->getOperand()) || !rst->getChild())
    return 0;
  if (!dynamic_cast<CallNode *>(rst->getChild()))
    return 0;
  return 3;
}
vector<TreeNode *> CallAssignTile::apply(TreeNode *node, TilingResult &result) const {
  auto rst = dynamic_cast<OperandNode *>(node);
  auto op = dynamic_cast<CallNode *>(rst->getChild());
  auto callee = op->getCallee();
  auto args = op->getArgs();
  vector<TreeNode *> edge = {callee, args};

  L2CodeBlockNode *block = new L2CodeBlockNode();
  result.addBlockRoot(block);

  for (auto eNode : edge)
    eNode->setCodeBlock(block);

  vector<string> argStrs;
  auto arguments = dynamic_cast<const Arguments *>(args->getOperand());
  for (auto arg : arguments->getArgs())
    argStrs.push_back(arg->toStr());

  auto insts = generateCall(rst->toStr(), argStrs);
  block->addInstructions(insts);
  return edge;
}
const CallAssignTile *CallAssignTile::getInstance() {
  if (!instance)
    instance = new CallAssignTile();
  return instance;
}
const CallAssignTile *CallAssignTile::instance = nullptr;

int LabelTile::match(TreeNode *node) const {
  if (!dynamic_cast<LabelNode *>(node))
    return 0;
  return 1;
}
vector<TreeNode *> LabelTile::apply(TreeNode *node, TilingResult &result) const {
  L2CodeBlockNode *block = new L2CodeBlockNode();
  result.addBlockRoot(block);

  auto insts = generateLabel(dynamic_cast<LabelNode *>(node)->getLabel()->toStr());
  block->addInstructions(insts);
  return {};
}
const LabelTile *LabelTile::getInstance() {
  if (!instance)
    instance = new LabelTile();
  return instance;
}
const LabelTile *LabelTile::instance = nullptr;

} // namespace L3