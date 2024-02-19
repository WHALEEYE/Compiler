#include <string>
#include <vector>

#include "liveness_analyzer.h"
#include <L3.h>
#include <helper.h>
#include <stdexcept>
#include <tree.h>

namespace L3 {

OperandNode::OperandNode(const Item *operand) : operand(operand), child(nullptr), status(MERGABLE) {}
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

/*
 * a context-wise tree constructor
 */
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

    disableMemAccess();
  }

  void visit(const StoreInst *inst) override {
    auto opNode = new StoreNode();
    opNode->setAddr(tryGetMergeNode(inst->getAddr()));
    opNode->setVal(tryGetMergeNode(inst->getVal()));
    this->node = opNode;

    disableMemAccess();
  }

  void visit(const ArithInst *inst) override {
    auto opNode = new ArithmeticNode();
    opNode->setLhs(tryGetMergeNode(inst->getLhs()));
    opNode->setRhs(tryGetMergeNode(inst->getRhs()));
    opNode->setOp(inst->getOp());
    auto node = new OperandNode(inst->getRst());
    node->setChild(opNode);
    this->node = node;
  }

  // insts that do not belong to conetxts
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

  void enterFunction(const LivenessResult &liveness) {
    this->changeContext();
    this->liveness = &liveness;
  }

  void changeContext() { this->definedVars.clear(); }

  OperandNode *tryGetMergeNode(const Item *item) {
    if (!dynamic_cast<const Variable *>(item))
      return new OperandNode(item);
    auto var = dynamic_cast<const Variable *>(item);
    if (definedVars.find(var) != definedVars.end() && definedVars[var]->getStatus() == OperandNode::Status::MERGABLE) {
      definedVars[var]->setStatus(OperandNode::Status::MERGED);
      return definedVars[var];
    }
    return new OperandNode(var);
  }

  void disableMemAccess() {
    for (auto &[var, node] : definedVars) {
      if (!dynamic_cast<const LoadNode *>(node->getChild()))
        continue;
      if (node->getStatus() != OperandNode::Status::MERGABLE)
        continue;
      node->setStatus(OperandNode::Status::UNMERGABLE);
    }
  }

  TreeNode *getNode() const { return node; }
  static TreeConstructor *getInstance() {
    if (!instance)
      instance = new TreeConstructor();
    return instance;
  }
  TreeConstructor(const TreeConstructor &) = delete;
  TreeConstructor &operator=(const TreeConstructor &) = delete;

private:
  TreeConstructor() = default;
  static TreeConstructor *instance;
  TreeNode *node{};

  bool defining = false;
  unordered_map<const Variable *, OperandNode *> definedVars;
  unordered_map<const Variable *, OperandNode *> usedVars;

  const LivenessResult *liveness{};
};
TreeConstructor *TreeConstructor::instance = nullptr;

const Trees &constructTrees(const Function *F, const LivenessResult &liveness) {
  auto constructor = TreeConstructor::getInstance();
  constructor->enterFunction(liveness);
  const Context *last = nullptr, *cxt;
  auto &trees = *(new Trees());
  for (auto BB : F->getBasicBlocks()) {
    for (auto I : BB->getInstructions()) {
      cxt = I->getContext();
      if (cxt != last)
        constructor->changeContext();
      last = cxt;

      debug("constructing tree for " + I->toStr());
      I->accept(*constructor);
      auto root = constructor->getNode();
      trees.push_back(root);
    }
  }
  return trees;
}

} // namespace L3