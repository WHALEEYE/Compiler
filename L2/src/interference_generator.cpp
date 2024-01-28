#include <L2.h>
#include <interference_generator.h>

namespace L2 {

InterferenceGraph::InterferenceGraph() {
  auto &allRegisters = Register::getAllRegisters();
  for (auto &reg : allRegisters) {
    graph[reg].insert(allRegisters.begin(), allRegisters.end());
    graph[reg].erase(reg);
  }
}

void generateInterferenceGraph(Function *F) {}

void printInterferenceGraph(Function *F) {
  // TODO
}

} // namespace L2