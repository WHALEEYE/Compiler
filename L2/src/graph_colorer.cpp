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

const std::unordered_map<const Symbol *, Register::ID> &ColorResult::getColorMap() const {
  return colorMap;
}
const SpillInfo &ColorResult::getSpillInfo() const { return *spillInfo; }
void ColorResult::dump() const {
  std::cout << "color map:" << std::endl;
  for (auto &[sym, color] : colorMap)
    std::cout << sym->toStr() << " " << Register::getRegister(color)->toStr() << std::endl;
  spillInfo->dump();
}

std::unordered_map<const Symbol *, Register::ID> colorMap;
SpillInfo *spillInfo;

enum class ColorResultType { SUCCESS, FAIL, CONTINUE };

const static auto K = 15;
const static auto colorPriority = {
    Register::ID::R10, Register::ID::R11, Register::ID::R8,  Register::ID::R9,  Register::ID::RAX,
    Register::ID::RCX, Register::ID::RDI, Register::ID::RDX, Register::ID::RSI, Register::ID::R12,
    Register::ID::R13, Register::ID::R14, Register::ID::R15, Register::ID::RBP, Register::ID::RBX};

/*
 * with interference graph, liveness result, Function pointer and spill info, try to color the graph
 * if succeeded, update the result
 * if spilled, update the result
 */
ColorResultType tryColor(Function *F, InterferenceGraph &interferenceGraph,
                         const LivenessResult &livenessResult, ColorResult &result) {
  auto &graph = interferenceGraph.graph;
  auto &colorMap = result.colorMap;
  auto &spillInfo = *result.spillInfo;
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
      for (auto nbr : graph[var]) {
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

  std::unordered_set<const Variable *> uncoloredVars;
  std::unordered_set<const Variable *> varsToBeSpilled;
  // gather all the nodes that are not colored
  for (auto &[sym, _] : graph) {
    if (colorMap.find(sym) == colorMap.end()) {
      auto var = dynamic_cast<const Variable *>(sym);
      uncoloredVars.insert(var);
      if (spillInfo.isSpilled(var))
        continue;
      varsToBeSpilled.insert(var);
    }
  }

  if (uncoloredVars.empty()) {
    return ColorResultType::SUCCESS;
  } else if (varsToBeSpilled.empty()) {
    return ColorResultType::FAIL;
  }
  // spill all the nodes that are not colored
  spillFunction(F, *result.spillInfo, livenessResult, varsToBeSpilled);
  return ColorResultType::CONTINUE;
}

const ColorResult &colorGraph(Function *F) {
  auto prefix = findSpillPrefix(F);
  auto spillInfo = new SpillInfo(prefix);

  bool failed = false;
  // a map recording the color (Register ID) of each node
  auto result = new ColorResult();
  result->spillInfo = spillInfo;
  // liveness analysis and interference graph construction
  const LivenessResult *livenessResult;
  InterferenceGraph *interferenceGraph;

  // BIG LOOP
  while (true) {
    livenessResult = &analyzeLiveness(F);
    interferenceGraph = &analyzeInterference(F, *livenessResult);
    auto colorResultType = tryColor(F, *interferenceGraph, *livenessResult, *result);
    if (colorResultType == ColorResultType::SUCCESS)
      break;
    else if (colorResultType == ColorResultType::FAIL) {
      failed = true;
      break;
    }
  }

  if (failed) {
    // spill all remaining variables
    // reuse the liveness result bcs it's not spilled since the last loop (failed bcs no variable
    // found can be spilled)
    std::unordered_set<const Variable *> varsToBeSpilled;
    for (auto &[sym, _] : interferenceGraph->graph) {
      if (auto var = dynamic_cast<const Variable *>(sym)) {
        if (spillInfo->isSpilled(var))
          continue;
        varsToBeSpilled.insert(var);
      }
    }
    spillFunction(F, *spillInfo, *livenessResult, varsToBeSpilled);

    // now, try coloring for the last time
    if (tryColor(F, *interferenceGraph, *livenessResult, *result) != ColorResultType::SUCCESS)
      throw std::runtime_error("spill failed");
  }

  return *result;
}
} // namespace L2