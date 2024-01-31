#pragma once
#include <map>

#include <L2.h>
#include <liveness_analyzer.h>
#include <spiller.h>
#include <unordered_set>

namespace L2 {

typedef std::map<const Symbol *, std::unordered_set<const Symbol *>> InterferenceGraph;

class InterferenceResult {
public:
  InterferenceResult() = default;
  const std::unordered_set<const Symbol *> &getNeighbors(const Symbol *s) const;
  const InterferenceGraph &getGraph() const;
  void dump() const;

  /*
   * Add an edge between two symbols.
   * If the symbols are the same, do nothing.
   */
  void addEdge(const Symbol *s1, const Symbol *s2);

private:
  InterferenceGraph graph;

  InterferenceResult &operator=(const InterferenceResult &) = delete;
  InterferenceResult(const InterferenceResult &) = delete;
};

InterferenceResult &analyzeInterference(const Function *F, const LivenessResult &livenessResult);

} // namespace L2