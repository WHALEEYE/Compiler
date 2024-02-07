#include <tree.h>

namespace L3 {

void TreeNode::setCodeBlock(L2CodeBlockNode *codeBlock) { this->codeBlock = codeBlock; }
L2CodeBlockNode *TreeNode::getCodeBlock() const { return codeBlock; }

OperandNode::OperandNode(const Item *operand) : operand(operand) {}
const Item *OperandNode::getOperand() const { return operand; }
OperationNode *OperandNode::getChild() { return child; }
string OperandNode::toStr() const { return operand->toStr(); }

} // namespace L3