#pragma once
#include <map>

#include <L2.h>

namespace L2 {

class InterferenceGraph {
public:
  InterferenceGraph() = default;
  void printGraph(Function *F);
  bool isInterfering(Symbol *s1, Symbol *s2);

private:
  void addEdge(Symbol *s1, Symbol *s2);
  void removeEdge(Symbol *s1, Symbol *s2);
  std::map<Symbol *, std::unordered_set<Symbol *>> graph;

  InterferenceGraph &operator=(const InterferenceGraph &) = delete;
  InterferenceGraph(const InterferenceGraph &) = delete;

  friend void generateInterferenceGraph(Function *F);
};



void generateInterferenceGraph(Function *F);
} // namespace L2