#include "loop_analyzer.h"
#include <LB.h>
#include <unordered_map>

namespace LB {

void generate_code(Program *P, std::unordered_map<const Function *,  const LoopInfo &>);

} // namespace LB
