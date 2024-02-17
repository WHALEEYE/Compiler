#include <stdexcept>
#include <unordered_set>
#include <vector>

#include <L3.h>
#include <code_generator.h>
#include <helper.h>
#include <string>

namespace L3 {

string CodeBlock::toStr() const {
  string ret;
  for (const auto &inst : instructions)
    ret += inst + "\n";
  return ret;
}

void CodeBlock::addInstructions(vector<string> &insts) {
  this->instructions.insert(this->instructions.end(), insts.begin(), insts.end());
}
const vector<string> &CodeBlock::getInstructions() const { return instructions; }
void CodeBlock::addChild(const CodeBlock *child) { children.insert(child); }
const unordered_set<const CodeBlock *> &CodeBlock::getChildren() const { return children; }

vector<string> assembleCodeRec(const CodeBlock *block) {
  vector<string> code;
  for (auto child : block->getChildren()) {
    auto childCode = assembleCodeRec(child);
    code.insert(code.end(), childCode.begin(), childCode.end());
  }
  auto selfCode = block->getInstructions();
  code.insert(code.end(), selfCode.begin(), selfCode.end());
  return code;
}

const vector<string> &TilingResult::assembleCode() const {
  auto &code = *(new vector<string>);
  for (auto root : roots) {
    auto rootCode = assembleCodeRec(root);
    code.insert(code.end(), rootCode.begin(), rootCode.end());
  }
  return code;
}

const unordered_set<const Tile *> &Tile::getTiles() { return tiles; }
const unordered_set<const Tile *> Tile::tiles = {
    ArithTile::getInstance(),     CompareTile::getInstance(), StoreTile::getInstance(),      LoadTile::getInstance(),
    AssignTile::getInstance(),    BranchTile::getInstance(),  CondBranchTile::getInstance(), ReturnTile::getInstance(),
    ReturnValTile::getInstance(), CallTile::getInstance(),    CallAssignTile::getInstance(), LabelTile::getInstance()};

void addBlock(const TreeNode *root, const vector<const TreeNode *> &leaves, CodeBlock *newBlock, TilingResult &result) {
  // get the block of the root node
  CodeBlock *prtBlock = nullptr;
  if (result.nodeToBlock.find(root) != result.nodeToBlock.end())
    prtBlock = result.nodeToBlock[root];
  if (prtBlock)
    prtBlock->addChild(newBlock);
  else
    result.roots.push_back(newBlock);

  for (auto leaf : leaves) {
    if (result.nodeToBlock.find(leaf) != result.nodeToBlock.end())
      throw runtime_error("Duplicate leaf");
    result.nodeToBlock[leaf] = newBlock;
  }
}

int ArithTile::match(const TreeNode *node) const {
  if (!dynamic_cast<const OperandNode *>(node))
    return 0;
  auto rst = dynamic_cast<const OperandNode *>(node);
  if (!dynamic_cast<const Variable *>(rst->getOperand()) || !rst->getChild())
    return 0;
  if (!dynamic_cast<const ArithmeticNode *>(rst->getChild()))
    return 0;

  return 3;
}
vector<const TreeNode *> ArithTile::apply(const TreeNode *node, TilingResult &result) const {
  auto rst = dynamic_cast<const OperandNode *>(node);
  auto op = dynamic_cast<const ArithmeticNode *>(rst->getChild());
  auto lhs = op->getLhs(), rhs = op->getRhs();
  vector<const TreeNode *> leaves = {lhs, rhs};

  auto *block = new CodeBlock();
  auto insts = generateArithmetic(rst, lhs, op, rhs);
  block->addInstructions(insts);
  addBlock(node, leaves, block, result);
  return leaves;
}
const ArithTile *ArithTile::getInstance() {
  if (!instance)
    instance = new ArithTile();
  return instance;
}
const ArithTile *ArithTile::instance = nullptr;

int CompareTile::match(const TreeNode *node) const {
  if (!dynamic_cast<const OperandNode *>(node))
    return 0;
  auto rst = dynamic_cast<const OperandNode *>(node);
  if (!dynamic_cast<const Variable *>(rst->getOperand()) || !rst->getChild())
    return 0;
  if (!dynamic_cast<const CompareNode *>(rst->getChild()))
    return 0;

  return 3;
}
vector<const TreeNode *> CompareTile::apply(const TreeNode *node, TilingResult &result) const {
  auto rst = dynamic_cast<const OperandNode *>(node);
  auto op = dynamic_cast<const CompareNode *>(rst->getChild());
  auto lhs = op->getLhs(), rhs = op->getRhs();
  vector<const TreeNode *> leaves = {lhs, rhs};
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

  auto *block = new CodeBlock();
  auto insts = generateCompare(rst_str, lhs_str, op_str, rhs_str);
  block->addInstructions(insts);
  addBlock(node, leaves, block, result);
  return leaves;
}
const CompareTile *CompareTile::getInstance() {
  if (!instance)
    instance = new CompareTile();
  return instance;
}
const CompareTile *CompareTile::instance = nullptr;

int StoreTile::match(const TreeNode *node) const {
  if (!dynamic_cast<const StoreNode *>(node))
    return 0;
  return 2;
}
vector<const TreeNode *> StoreTile::apply(const TreeNode *node, TilingResult &result) const {
  auto storeNode = dynamic_cast<const StoreNode *>(node);
  auto addr = storeNode->getAddr();
  auto val = storeNode->getVal();
  vector<const TreeNode *> leaves = {val};

  auto *block = new CodeBlock();
  auto insts = generateStore(addr->toStr(), val->toStr());
  block->addInstructions(insts);
  addBlock(node, leaves, block, result);
  return leaves;
}
const StoreTile *StoreTile::getInstance() {
  if (!instance)
    instance = new StoreTile();
  return instance;
}
const StoreTile *StoreTile::instance = nullptr;

int LoadTile::match(const TreeNode *node) const {
  if (!dynamic_cast<const OperandNode *>(node))
    return 0;
  auto lvalNode = dynamic_cast<const OperandNode *>(node);
  if (!dynamic_cast<const Variable *>(lvalNode->getOperand()) || !lvalNode->getChild())
    return 0;
  if (!dynamic_cast<const LoadNode *>(lvalNode->getChild()))
    return 0;
  return 2;
}
vector<const TreeNode *> LoadTile::apply(const TreeNode *node, TilingResult &result) const {
  auto val = dynamic_cast<const OperandNode *>(node);
  auto op = dynamic_cast<const LoadNode *>(val->getChild());
  auto addr = op->getAddr();
  vector<const TreeNode *> leaves = {addr};

  auto *block = new CodeBlock();
  auto insts = generateLoad(val->toStr(), addr->toStr());
  block->addInstructions(insts);
  addBlock(node, leaves, block, result);
  return leaves;
}
const LoadTile *LoadTile::getInstance() {
  if (!instance)
    instance = new LoadTile();
  return instance;
}
const LoadTile *LoadTile::instance = nullptr;

int AssignTile::match(const TreeNode *node) const {
  if (!dynamic_cast<const OperandNode *>(node))
    return 0;
  auto lvalNode = dynamic_cast<const OperandNode *>(node);
  if (!dynamic_cast<const Variable *>(lvalNode->getOperand()) || !lvalNode->getChild())
    return 0;
  if (!dynamic_cast<const AssignNode *>(lvalNode->getChild()))
    return 0;
  return 2;
}
vector<const TreeNode *> AssignTile::apply(const TreeNode *node, TilingResult &result) const {
  auto lhs = dynamic_cast<const OperandNode *>(node);
  auto op = dynamic_cast<const AssignNode *>(lhs->getChild());
  auto rhs = op->getRhs();
  vector<const TreeNode *> leaves = {rhs};

  auto *block = new CodeBlock();
  auto insts = generateAssign(lhs->toStr(), rhs->toStr());
  block->addInstructions(insts);
  addBlock(node, leaves, block, result);
  return leaves;
}
const AssignTile *AssignTile::getInstance() {
  if (!instance)
    instance = new AssignTile();
  return instance;
}
const AssignTile *AssignTile::instance = nullptr;

int BranchTile::match(const TreeNode *node) const {
  if (!dynamic_cast<const BranchNode *>(node))
    return 0;
  return 1;
}
vector<const TreeNode *> BranchTile::apply(const TreeNode *node, TilingResult &result) const {
  auto op = dynamic_cast<const BranchNode *>(node);
  auto label = op->getLabel();

  auto *block = new CodeBlock();
  auto insts = generateBranch(label->toStr());
  block->addInstructions(insts);
  addBlock(node, {}, block, result);
  return {};
}
const BranchTile *BranchTile::getInstance() {
  if (!instance)
    instance = new BranchTile();
  return instance;
}
const BranchTile *BranchTile::instance = nullptr;

int CondBranchTile::match(const TreeNode *node) const {
  if (!dynamic_cast<const CondBranchNode *>(node))
    return 0;
  return 2;
}
vector<const TreeNode *> CondBranchTile::apply(const TreeNode *node, TilingResult &result) const {
  auto op = dynamic_cast<const CondBranchNode *>(node);
  auto cond = op->getCond(), label = op->getLabel();
  vector<const TreeNode *> leaves = {cond};

  auto *block = new CodeBlock();
  auto insts = generateCondBranch(cond->toStr(), label->toStr());
  block->addInstructions(insts);
  addBlock(node, leaves, block, result);
  return leaves;
}
const CondBranchTile *CondBranchTile::getInstance() {
  if (!instance)
    instance = new CondBranchTile();
  return instance;
}
const CondBranchTile *CondBranchTile::instance = nullptr;

int ReturnTile::match(const TreeNode *node) const {
  if (!dynamic_cast<const ReturnNode *>(node))
    return 0;
  return 1;
}
vector<const TreeNode *> ReturnTile::apply(const TreeNode *node, TilingResult &result) const {
  auto *block = new CodeBlock();
  auto insts = generateReturn();
  block->addInstructions(insts);
  addBlock(node, {}, block, result);
  return {};
}
const ReturnTile *ReturnTile::getInstance() {
  if (!instance)
    instance = new ReturnTile();
  return instance;
}
const ReturnTile *ReturnTile::instance = nullptr;

int ReturnValTile::match(const TreeNode *node) const {
  if (!dynamic_cast<const ReturnValNode *>(node))
    return 0;
  return 1;
}
vector<const TreeNode *> ReturnValTile::apply(const TreeNode *node, TilingResult &result) const {
  auto op = dynamic_cast<const ReturnValNode *>(node);
  auto val = op->getVal();
  vector<const TreeNode *> leaves = {val};

  auto *block = new CodeBlock();
  auto insts = generateReturnVal(val->toStr());
  block->addInstructions(insts);
  addBlock(node, leaves, block, result);
  return leaves;
}
const ReturnValTile *ReturnValTile::getInstance() {
  if (!instance)
    instance = new ReturnValTile();
  return instance;
}
const ReturnValTile *ReturnValTile::instance = nullptr;

int CallTile::match(const TreeNode *node) const {
  if (!dynamic_cast<const CallNode *>(node))
    return 0;
  return 2;
}
vector<const TreeNode *> CallTile::apply(const TreeNode *node, TilingResult &result) const {
  auto op = dynamic_cast<const CallNode *>(node);
  auto callee = op->getCallee();
  auto args = op->getArgs();

  auto *block = new CodeBlock();
  vector<string> argStrs;
  auto arguments = dynamic_cast<const Arguments *>(args->getOperand());
  for (auto arg : arguments->getArgs())
    argStrs.push_back(arg->toStr());

  auto insts = generateCall(callee->toStr(), argStrs);
  block->addInstructions(insts);
  addBlock(node, {}, block, result);
  return {};
}
const CallTile *CallTile::getInstance() {
  if (!instance)
    instance = new CallTile();
  return instance;
}
const CallTile *CallTile::instance = nullptr;

int CallAssignTile::match(const TreeNode *node) const {
  if (!dynamic_cast<const OperandNode *>(node))
    return 0;
  auto rst = dynamic_cast<const OperandNode *>(node);
  if (!dynamic_cast<const Variable *>(rst->getOperand()) || !rst->getChild())
    return 0;
  if (!dynamic_cast<const CallNode *>(rst->getChild()))
    return 0;
  return 3;
}
vector<const TreeNode *> CallAssignTile::apply(const TreeNode *node, TilingResult &result) const {
  auto rst = dynamic_cast<const OperandNode *>(node);
  auto op = dynamic_cast<const CallNode *>(rst->getChild());
  auto callee = op->getCallee();
  auto args = op->getArgs();

  auto *block = new CodeBlock();
  vector<string> argStrs;
  auto arguments = dynamic_cast<const Arguments *>(args->getOperand());
  for (auto arg : arguments->getArgs())
    argStrs.push_back(arg->toStr());

  auto insts = generateCallAssign(rst->toStr(), callee->toStr(), argStrs);
  block->addInstructions(insts);
  addBlock(node, {}, block, result);
  return {};
}
const CallAssignTile *CallAssignTile::getInstance() {
  if (!instance)
    instance = new CallAssignTile();
  return instance;
}
const CallAssignTile *CallAssignTile::instance = nullptr;

int LabelTile::match(const TreeNode *node) const {
  if (!dynamic_cast<const LabelNode *>(node))
    return 0;
  return 1;
}
vector<const TreeNode *> LabelTile::apply(const TreeNode *node, TilingResult &result) const {
  auto *block = new CodeBlock();
  vector<string> label = {dynamic_cast<const LabelNode *>(node)->toStr()};
  block->addInstructions(label);
  addBlock(node, {}, block, result);
  return {};
}
const LabelTile *LabelTile::getInstance() {
  if (!instance)
    instance = new LabelTile();
  return instance;
}
const LabelTile *LabelTile::instance = nullptr;

const TilingResult &tileFunction(const Trees &trees) {
  auto result = new TilingResult();

  vector<const TreeNode *> subRoots = trees;
  const TreeNode *curr;
  auto &tiles = Tile::getTiles();
  while (!subRoots.empty()) {
    curr = subRoots.front();
    subRoots.erase(subRoots.begin());

    debug("Tiling node: " + curr->toStr());

    int maxCost = 0;
    const Tile *maxTile = nullptr;
    for (auto tile : tiles) {
      int cost = tile->match(curr);
      if (cost > maxCost) {
        maxCost = cost;
        maxTile = tile;
      }
    }

    if (maxCost == 0)
      throw runtime_error("No tile matched");

    auto leaves = maxTile->apply(curr, *result);
    for (auto node : leaves) {
      if (!dynamic_cast<const OperandNode *>(node))
        throw runtime_error("Invalid edge node");
      if (dynamic_cast<const OperandNode *>(node)->getChild())
        subRoots.push_back(node);
    }
  }

  return *result;
}

} // namespace L3