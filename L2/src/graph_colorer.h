#pragma once

#include <L2.h>
#include <interference_analyzer.h>
#include <liveness_analyzer.h>
#include <spiller.h>

namespace L2 {

typedef std::unordered_map<const Symbol *, Register::ID> ColorMap;

class ColorResult {
public:
  const ColorMap &getColorMap() const;
  const SpillInfo &getSpillInfo() const;
  void dump() const;

private:
  ColorMap colorMap;
  SpillInfo *spillInfo;

  friend const ColorResult &colorGraph(Function *F);
  friend bool tryColor(Function *F, ColorResult &result);
};

const ColorResult &colorGraph(Function *F);
} // namespace L2