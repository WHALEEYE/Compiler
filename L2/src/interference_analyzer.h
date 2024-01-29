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
  bool hasEdge(const Symbol *s1, const Symbol *s2);

private:
  void addEdge(const Symbol *s1, const Symbol *s2);
  void removeEdge(const Symbol *s1, const Symbol *s2);
  std::map<const Symbol *, std::unordered_set<const Symbol *>> graph;

  InterferenceGraph &operator=(const InterferenceGraph &) = delete;
  InterferenceGraph(const InterferenceGraph &) = delete;

  friend const InterferenceResult &analyzeInterference(const Program *P,
                                                       const LivenessResult &livenessResult);
};

class InterferenceResult {
public:
  InterferenceResult() = default;
  const InterferenceGraph &getFunctionGraph(const Function *F) const;

private:
  std::map<const Function *, InterferenceGraph> functionGraphs;

  InterferenceResult &operator=(const InterferenceResult &) = delete;
  InterferenceResult(const InterferenceResult &) = delete;

  friend const InterferenceResult &analyzeInterference(const Program *P,
                                                       const LivenessResult &livenessResult);
};

const InterferenceResult &analyzeInterference(const Program *P,
                                              const LivenessResult &livenessResult);

} // namespace L2