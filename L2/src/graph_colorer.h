#pragma once

#include <L2.h>
#include <interference_analyzer.h>
#include <liveness_analyzer.h>
#include <spiller.h>

namespace L2 {
class ColorResult {
public:
  const std::unordered_map<const Symbol *, Register::ID> &getColorMap() const;
  const SpillInfo &getSpillInfo() const;
  void dump() const;

private:
  std::unordered_map<const Symbol *, Register::ID> colorMap;
  SpillInfo *spillInfo;

  friend const ColorResult &colorGraph(Function *F);
  friend ColorResultType tryColor(Function *F, InterferenceGraph &interferenceGraph,
                                  const LivenessResult &livenessResult, ColorResult &result);
};

const ColorResult &colorGraph(Function *F);
} // namespace L2