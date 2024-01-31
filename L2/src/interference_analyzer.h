#pragma once
#include <map>

#include <L2.h>
#include <liveness_analyzer.h>
#include <spiller.h>
#include <unordered_set>

namespace L2 {

class ColorResult;
enum class ColorResultType;

class InterferenceGraph {
public:
  InterferenceGraph() = default;
  void dump() const;
  bool hasEdge(const Symbol *s1, const Symbol *s2);
  const std::unordered_set<const Symbol *> &getNeighbors(const Symbol *s) const;

private:
  void addEdge(const Symbol *s1, const Symbol *s2);
  void removeEdge(const Symbol *s1, const Symbol *s2);
  std::map<const Symbol *, std::unordered_set<const Symbol *>> graph;

  InterferenceGraph &operator=(const InterferenceGraph &) = delete;
  InterferenceGraph(const InterferenceGraph &) = delete;

  friend InterferenceGraph &analyzeInterference(const Function *F,
                                                const LivenessResult &livenessResult);
  friend const ColorResult &colorGraph(Function *F);
  friend ColorResultType tryColor(Function *F, InterferenceGraph &interferenceGraph,
                         const LivenessResult &livenessResult, ColorResult &result);
};

InterferenceGraph &analyzeInterference(const Function *F, const LivenessResult &livenessResult);

} // namespace L2