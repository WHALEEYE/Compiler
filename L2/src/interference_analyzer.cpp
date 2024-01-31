#include <iostream>

#include <L2.h>
#include <interference_analyzer.h>
#include <unordered_set>

namespace L2 {

const std::unordered_set<const Symbol *> &InterferenceResult::getNeighbors(const Symbol *s) const {
  return graph.at(s);
}
const InterferenceGraph &InterferenceResult::getGraph() const { return graph; }

void InterferenceResult::addEdge(const Symbol *s1, const Symbol *s2) {
  // if node is not in graph, add it
  if (graph.find(s1) == graph.end())
    graph[s1] = std::unordered_set<const Symbol *>();
  if (graph.find(s2) == graph.end())
    graph[s2] = std::unordered_set<const Symbol *>();

  if (s1 == s2)
    return;

  graph[s1].insert(s2);
  graph[s2].insert(s1);
}

void InterferenceResult::dump() const {
  for (auto &[symbol, intSyms] : graph) {
    std::cout << symbol->toStr() << " ";

    for (auto intSym : intSyms)
      std::cout << intSym->toStr() << " ";

    std::cout << std::endl;
  }
}

InterferenceResult &analyzeInterference(const Function *F, const LivenessResult &livenessResult) {
  auto *interferenceGraph = new InterferenceResult();
  auto &allGPRegisters = Register::getAllGPRegisters();

  // connect all GP registers
  for (auto reg1 : allGPRegisters)
    for (auto reg2 : allGPRegisters)
      interferenceGraph->addEdge(reg1, reg2);

  for (auto BB : F->getBasicBlocks()) {
    for (auto I : BB->getInstructions()) {
      auto &livenessSets = livenessResult.getLivenessSets(I);
      auto &IN = livenessSets.getIN(), &OUT = livenessSets.getOUT(), &KILL = livenessSets.getKILL();

      for (auto sym1 : IN)
        for (auto sym2 : IN)
          interferenceGraph->addEdge(sym1, sym2);

      for (auto sym1 : OUT)
        for (auto sym2 : OUT)
          interferenceGraph->addEdge(sym1, sym2);

      for (auto killSym : KILL)
        for (auto outSym : OUT)
          interferenceGraph->addEdge(killSym, outSym);

      // add instruction specific edges
      if (auto shiftInst = dynamic_cast<const ShiftInst *>(I))
        if (auto rVal = dynamic_cast<const Symbol *>(shiftInst->getRval()))
          for (auto reg : allGPRegisters)
            if (reg->getID() != Register::ID::RCX)
              interferenceGraph->addEdge(rVal, reg);
    }
  }

  return *interferenceGraph;
}

} // namespace L2