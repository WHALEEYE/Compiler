#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include <L2.h>
#include <graph_colorer.h>
#include <interference_analyzer.h>
#include <liveness_analyzer.h>
#include <spiller.h>

namespace L2 {

std::string findSpillPrefix(Function *F) {
  std::string preFix = "%a";
  auto alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int ind = 1, cur = 0, max = 52;
  auto &variables = F->getVariables();
  bool hasPrefix;
  do {
    hasPrefix = false;
    for (auto &[varName, _] : variables) {
      if (varName.compare(0, preFix.size(), preFix) == 0) {
        hasPrefix = true;
        if (cur == max) {
          ind++;
          cur = 0;
          preFix += alphabet[0];
        } else {
          preFix[ind] = alphabet[cur++];
        }
        break;
      }
    }
  } while (hasPrefix);
  return preFix;
}

typedef struct Node {
  const Variable *var;
  int degree;
} Node;

bool inGraph(const std::unordered_set<const Variable *> &removed, const Symbol *sym) {
  if (auto var = dynamic_cast<const Variable *>(sym))
    return removed.find(var) == removed.end();
  return true;
}

int getDegree(const std::unordered_set<const Variable *> &removed,
              const std::unordered_set<const Symbol *> &neighbors) {
  int count = 0;

  for (auto nbr : neighbors)
    if (inGraph(removed, nbr))
      count++;

  return count;
}

const ColorMap &ColorResult::getColorMap() const { return colorMap; }
const SpillInfo &ColorResult::getSpillInfo() const { return *spillInfo; }
void ColorResult::dump() const {
  std::cout << "color map:" << std::endl;
  for (auto &[sym, color] : colorMap)
    std::cout << sym->toStr() << " " << Register::getRegister(color)->toStr() << std::endl;
  spillInfo->dump();
}

const static auto K = 15;
const static auto colorPriority = {
    Register::ID::R10, Register::ID::R11, Register::ID::R8,  Register::ID::R9,  Register::ID::RAX,
    Register::ID::RCX, Register::ID::RDI, Register::ID::RDX, Register::ID::RSI, Register::ID::R12,
    Register::ID::R13, Register::ID::R14, Register::ID::R15, Register::ID::RBP, Register::ID::RBX};

/*
 * Try to color the graph.
 * This function will update the result passed in.
 */
bool tryColor(Function *F, ColorResult &result) {
  auto &livenessResult = analyzeLiveness(F);
  auto &interferenceResult = analyzeInterference(F, livenessResult);
  auto &graph = interferenceResult.getGraph();

  auto &spillInfo = *result.spillInfo;
  auto &colorMap = result.colorMap;
  colorMap.clear();
  // stack
  std::vector<const Variable *> stack;
  std::unordered_set<const Variable *> removed;

  colorMap.clear();
  for (auto reg : Register::getAllGPRegisters())
    colorMap[reg] = reg->getID();

  // remove nodes with edges < K
  bool stop;

  do {
    stop = true;
    for (auto &[sym, neighbors] : graph) {
      if (!dynamic_cast<const Variable *>(sym))
        continue;
      if (!inGraph(removed, sym))
        continue;
      auto var = dynamic_cast<const Variable *>(sym);
      if (getDegree(removed, neighbors) < K) {
        stop = false;
        stack.push_back(var);
        removed.insert(var);
      }
    }
  } while (!stop);

  // remove other nodes
  std::vector<Node> nodes;
  for (auto &[sym, neighbors] : graph) {
    if (!inGraph(removed, sym))
      continue;
    // don't care about registers, we do not color them
    if (auto var = dynamic_cast<const Variable *>(sym))
      nodes.push_back({var, getDegree(removed, neighbors)});
  }
  std::sort(nodes.begin(), nodes.end(),
            [](const Node &a, const Node &b) { return a.degree > b.degree; });
  for (auto &node : nodes) {
    stack.push_back(node.var);
    removed.insert(node.var);
  }

  while (!stack.empty()) {
    // pop a node from the stack
    auto var = stack.back();
    stack.pop_back();
    removed.erase(var);

    // assign a color for it if possible
    for (auto color : colorPriority) {
      bool legal = true;
      for (auto nbr : interferenceResult.getNeighbors(var)) {
        if (colorMap.find(nbr) != colorMap.end() && colorMap.at(nbr) == color) {
          legal = false;
          break;
        }
      }
      if (legal) {
        colorMap[var] = color;
        break;
      }
    }
  }

  std::unordered_set<const Variable *> uncoloredVars, varsToBeSpilled, unspilledVars;
  bool spilled, colored;
  // gather all the nodes that are not colored
  for (auto &[sym, _] : graph) {
    if (!dynamic_cast<const Variable *>(sym))
      continue;
    auto var = dynamic_cast<const Variable *>(sym);
    spilled = spillInfo.isSpilled(var);
    colored = colorMap.find(var) != colorMap.end();
    if (!spilled && !colored)
      varsToBeSpilled.insert(var);
    if (!colored)
      uncoloredVars.insert(var);
    if (!spilled)
      unspilledVars.insert(var);
  }

  if (uncoloredVars.empty()) // all the variables are colored
    return true;
  else if (unspilledVars.empty()) // uncolored vars remaining, but all the variables are spilled
    throw std::runtime_error("failed to color the graph");
  else if (varsToBeSpilled.empty()) // spill all the nodes that are not spilled
    varsToBeSpilled = unspilledVars;

  spillFunction(F, *result.spillInfo, livenessResult, varsToBeSpilled);
  return false;
}

const ColorResult &colorGraph(Function *F) {
  auto prefix = findSpillPrefix(F);
  auto spillInfo = new SpillInfo(prefix);

  auto &result = *(new ColorResult());
  result.spillInfo = spillInfo;

  while (true)
    if (tryColor(F, result))
      return result;
}

} // namespace L2