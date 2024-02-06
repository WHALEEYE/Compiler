#include <L3.h>
#include <tile.h>
#include <tree.h>

namespace L3 {

const std::unordered_map<Tile::Type, const Tile &> Tile::enumMap = {
    {Type::ARITH, ArithTile::getInstance()}};

int ArithTile::match(TreeNode *node) {
  if (!dynamic_cast<VariableNode *>(node))
    return 0;
  auto varNode = dynamic_cast<VariableNode *>(node);
  if (!dynamic_cast<OperationNode *>(varNode->getChild()))
    return 0;
  auto opNode = dynamic_cast<OperationNode *>(varNode->getChild());
  auto op = opNode->getOperation();
  switch (op) {
  case OperationNode::Operation::ADD:
  case OperationNode::Operation::SUB:
  case OperationNode::Operation::MUL:
  case OperationNode::Operation::AND:
  case OperationNode::Operation::LEFT_SHIFT:
  case OperationNode::Operation::RIGHT_SHIFT:
    break;
  default:
    return 0;
  }
  auto l = opNode->getLeft(), r = opNode->getRight();
  if (!dynamic_cast<const Value *>(l->getVariable()) ||
      !dynamic_cast<const Value *>(r->getVariable()))
    return 0;

  node->setMatchedTile(this, false);
  opNode->setMatchedTile(this, false);
  l->setMatchedTile(this, true);
  r->setMatchedTile(this, true);
  return 3;
}

} // namespace L3