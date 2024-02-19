#include <string>
#include <vector>

#include <L3.h>
#include <helper.h>
#include <tree.h>

namespace L3 {

void TreeNode::setContext(TreeContext *context) { this->context = context; }

OperandNode::OperandNode(const Item *operand) : operand(operand), child(nullptr) {}
const Item *OperandNode::getOperand() const { return operand; }
const OperationNode *OperandNode::getChild() const { return child; }
void OperandNode::setChild(OperationNode *child) { this->child = child; }
string OperandNode::toStr() const { return operand->toStr(); }

void CallNode::setCallee(const OperandNode *callee) { this->callee = callee; }
const OperandNode *CallNode::getCallee() const { return callee; }
void CallNode::setArgs(const OperandNode *args) { this->args = args; }
const OperandNode *CallNode::getArgs() const { return args; }
string CallNode::toStr() const { return "call"; }

string ReturnNode::toStr() const { return "return"; }

void ReturnValNode::setVal(const OperandNode *val) { this->val = val; }
const OperandNode *ReturnValNode::getVal() const { return val; }
string ReturnValNode::toStr() const { return "return val"; }

void AssignNode::setRhs(const OperandNode *rhs) { this->rhs = rhs; }
const OperandNode *AssignNode::getRhs() const { return rhs; }
string AssignNode::toStr() const { return "<-"; }

void CompareNode::setLhs(const OperandNode *lhs) { this->lhs = lhs; }
const OperandNode *CompareNode::getLhs() const { return lhs; }
void CompareNode::setRhs(const OperandNode *rhs) { this->rhs = rhs; }
const OperandNode *CompareNode::getRhs() const { return rhs; }
void CompareNode::setOp(const CompareOp *op) { this->op = op; }
const CompareOp *CompareNode::getOp() const { return op; }
string CompareNode::toStr() const { return this->op->toStr(); }

void LoadNode::setAddr(const OperandNode *addr) { this->addr = addr; }
const OperandNode *LoadNode::getAddr() const { return addr; }
string LoadNode::toStr() const { return "load"; }

void StoreNode::setVal(const OperandNode *val) { this->val = val; }
const OperandNode *StoreNode::getVal() const { return val; }
string StoreNode::toStr() const { return "store"; }

void ArithmeticNode::setLhs(const OperandNode *lhs) { this->lhs = lhs; }
const OperandNode *ArithmeticNode::getLhs() const { return lhs; }
void ArithmeticNode::setRhs(const OperandNode *rhs) { this->rhs = rhs; }
const OperandNode *ArithmeticNode::getRhs() const { return rhs; }
void ArithmeticNode::setOp(const ArithOp *op) { this->op = op; }
const ArithOp *ArithmeticNode::getOp() const { return op; }
string ArithmeticNode::toStr() const { return this->op->toStr(); }

void BranchNode::setLabel(const OperandNode *label) { this->label = label; }
const OperandNode *BranchNode::getLabel() const { return label; }
string BranchNode::toStr() const { return "branch"; }

void CondBranchNode::setCond(const OperandNode *cond) { this->cond = cond; }
const OperandNode *CondBranchNode::getCond() const { return cond; }
void CondBranchNode::setLabel(const OperandNode *label) { this->label = label; }
const OperandNode *CondBranchNode::getLabel() const { return label; }
string CondBranchNode::toStr() const { return "cbranch"; }

void LabelNode::setLabel(const Label *label) { this->label = label; }
const Label *LabelNode::getLabel() const { return label; }
string LabelNode::toStr() const { return label->toStr(); }

void TreeContext::addTreeRoot(TreeNode *root) { this->treeRoots.push_back(root); }
const vector<const TreeNode *> &TreeContext::getTreeRoots() const { return treeRoots; }

class TreeConstructor : public Visitor {
public:
  void visit(const Variable *var) override {}
  void visit(const Number *num) override {}
  void visit(const Arguments *args) override {}
  void visit(const Parameters *params) override {}
  void visit(const CompareOp *op) override {}
  void visit(const ArithOp *op) override {}
  void visit(const RuntimeFunction *func) override {}
  void visit(const FunctionName *name) override {}
  void visit(const Label *label) override {}

  void visit(const CallInst *inst) override {
    auto opNode = new CallNode();
    opNode->setCallee(new OperandNode(inst->getCallee()));
    opNode->setArgs(new OperandNode(inst->getArgs()));
    this->node = opNode;
  }

  void visit(const CallAssignInst *inst) override {
    auto opNode = new CallNode();
    opNode->setCallee(new OperandNode(inst->getCallee()));
    opNode->setArgs(new OperandNode(inst->getArgs()));
    auto node = new OperandNode(inst->getRst());
    node->setChild(opNode);
    this->node = node;
  }

  void visit(const LabelInst *inst) override {
    auto opNode = new LabelNode();
    opNode->setLabel(inst->getLabel());
    this->node = opNode;
  }

  void visit(const RetInst *inst) override {
    auto opNode = new ReturnNode();
    this->node = opNode;
  }

  void visit(const RetValueInst *inst) override {
    auto opNode = new ReturnValNode();
    opNode->setVal(new OperandNode(inst->getVal()));
    this->node = opNode;
  }

  void visit(const AssignInst *inst) override {
    auto opNode = new AssignNode();
    opNode->setRhs(new OperandNode(inst->getRhs()));
    auto node = new OperandNode(inst->getLhs());
    node->setChild(opNode);
    this->node = node;
  }

  void visit(const CompareInst *inst) override {
    auto opNode = new CompareNode();
    opNode->setLhs(new OperandNode(inst->getLhs()));
    opNode->setRhs(new OperandNode(inst->getRhs()));
    opNode->setOp(inst->getOp());
    auto node = new OperandNode(inst->getRst());
    node->setChild(opNode);
    this->node = node;
  }

  void visit(const LoadInst *inst) override {
    auto opNode = new LoadNode();
    opNode->setAddr(new OperandNode(inst->getAddr()));
    auto node = new OperandNode(inst->getVal());
    node->setChild(opNode);
    this->node = node;
  }

  void visit(const StoreInst *inst) override {
    auto opNode = new StoreNode();
    opNode->setVal(new OperandNode(inst->getVal()));
    auto node = new OperandNode(inst->getAddr());
    node->setChild(opNode);
    this->node = node;
  }

  void visit(const ArithInst *inst) override {
    auto opNode = new ArithmeticNode();
    opNode->setLhs(new OperandNode(inst->getLhs()));
    opNode->setRhs(new OperandNode(inst->getRhs()));
    opNode->setOp(inst->getOp());
    auto node = new OperandNode(inst->getRst());
    node->setChild(opNode);
    this->node = node;
  }

  void visit(const BranchInst *inst) override {
    auto opNode = new BranchNode();
    opNode->setLabel(new OperandNode(inst->getLabel()));
    this->node = opNode;
  }

  void visit(const CondBranchInst *inst) override {
    auto opNode = new CondBranchNode();
    opNode->setCond(new OperandNode(inst->getCondition()));
    opNode->setLabel(new OperandNode(inst->getLabel()));
    this->node = opNode;
  }

  TreeNode *getNode() const { return node; }
  static TreeConstructor *getInstance() {
    if (!instance)
      instance = new TreeConstructor();
    return instance;
  }

private:
  TreeConstructor() = default;
  TreeConstructor(const TreeConstructor &) = delete;
  TreeConstructor &operator=(const TreeConstructor &) = delete;

  TreeNode *node;
  static TreeConstructor *instance;
};
TreeConstructor *TreeConstructor::instance = nullptr;

const vector<const TreeNode *> &constructTreesInFunc(const Function *F) {
  auto constructor = TreeConstructor::getInstance();
  const Context *last = nullptr, *cxt;
  TreeContext *curr;
  auto &roots = *(new vector<const TreeNode *>());
  for (auto I : F->getInstructions()) {
    debug("constructing tree for " + I->toStr());
    I->accept(*constructor);
    auto root = constructor->getNode();
    cxt = I->getContext();
    if (cxt) {
      if (cxt != last)
        curr = new TreeContext();

      root->setContext(curr);
      curr->addTreeRoot(root);
    } else
      root->setContext(nullptr);

    roots.push_back(root);
    last = cxt;
  }
  return roots;
}

const TreeResult &constructTrees(const Program *P) {
  auto &result = *(new TreeResult());
  for (auto F : P->getFunctions())
    result.insert({F, constructTreesInFunc(F)});
  return result;
}
} // namespace L3