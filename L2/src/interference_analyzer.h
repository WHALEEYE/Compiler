#pragma once
#include <map>

#include <L2.h>
#include <liveness_analyzer.h>

namespace L2 {

class InterferenceResult;

class InterferenceGraph {
public:
  InterferenceGraph() = default;
  void dump() const;
  bool hasEdge(Symbol *s1, Symbol *s2);

private:
  void addEdge(Symbol *s1, Symbol *s2);
  void removeEdge(Symbol *s1, Symbol *s2);
  std::map<Symbol *, std::unordered_set<Symbol *>> graph;

  InterferenceGraph &operator=(const InterferenceGraph &) = delete;
  InterferenceGraph(const InterferenceGraph &) = delete;

  friend const InterferenceResult &analyzeInterference(Program &P,
                                                       const LivenessResult &livenessResult);
};

class InterferenceResult {
public:
  InterferenceResult() = default;
  const InterferenceGraph &getFunctionGraph(Function *F) const;

private:
  std::map<Function *, InterferenceGraph> functionGraphs;

  InterferenceResult &operator=(const InterferenceResult &) = delete;
  InterferenceResult(const InterferenceResult &) = delete;

  friend const InterferenceResult &analyzeInterference(Program &P,
                                                       const LivenessResult &livenessResult);
};

const InterferenceResult &analyzeInterference(Program &P, const LivenessResult &livenessResult);

} // namespace L2