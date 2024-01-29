#include <iostream>

#include <L2.h>
#include <interference_analyzer.h>

namespace L2 {

void InterferenceGraph::dump() const {
  for (auto &[symbol, intSyms] : graph) {
    std::cout << symbol->toStr() << " ";

    for (auto intSym : intSyms)
      std::cout << intSym->toStr() << " ";

    std::cout << std::endl;
  }
}

bool InterferenceGraph::hasEdge(const Symbol *s1, const Symbol *s2) {
  if (graph.find(s1) == graph.end())
    return false;

  return graph.at(s1).find(s2) != graph.at(s1).end();
}

void InterferenceGraph::addEdge(const Symbol *s1, const Symbol *s2) {
  if (s1 == s2)
    return;

  graph[s1].insert(s2);
  graph[s2].insert(s1);
}

const InterferenceGraph &InterferenceResult::getFunctionGraph(const Function *F) const {
  return functionGraphs.at(F);
}

const InterferenceResult &analyzeInterference(const Program *P, const LivenessResult &livenessResult) {
  auto *interferenceResult = new InterferenceResult();
  auto &allGPRegisters = Register::getAllGPRegisters();
  for (auto F : P->getFunctions()) {
    auto &functionGraph = interferenceResult->functionGraphs[F];
    auto &graph = functionGraph.graph;
    auto &functionLivenessResult = livenessResult.getFunctionResult(F);

    // connect all GP registers
    for (auto reg : allGPRegisters) {
      graph[reg].insert(allGPRegisters.begin(), allGPRegisters.end());
      graph[reg].erase(reg);
    }

    for (auto BB : F->getBasicBlocks()) {
      for (auto I : BB->getInstructions()) {
        auto &livenessSets = functionLivenessResult.getLivenessSets(I);
        auto &IN = livenessSets.getIN(), &OUT = livenessSets.getOUT(),
             &KILL = livenessSets.getKILL();

        for (auto inSym : IN) {
          graph[inSym].insert(IN.begin(), IN.end());
          graph[inSym].erase(inSym);
        }

        for (auto outSym : OUT) {
          graph[outSym].insert(OUT.begin(), OUT.end());
          graph[outSym].erase(outSym);
        }

        for (auto killSym : KILL)
          for (auto outSym : OUT)
            functionGraph.addEdge(killSym, outSym);

        // add instruction specific edges
        if (auto shiftInst = dynamic_cast<const ShiftInst *>(I))
          if (auto rVal = dynamic_cast<const Symbol *>(shiftInst->getRval()))
            for (auto reg : allGPRegisters)
              if (reg != Register::getRegister(Register::ID::RCX))
                functionGraph.addEdge(rVal, reg);
      }
    }
  }
  return *interferenceResult;
}

} // namespace L2