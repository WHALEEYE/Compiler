#pragma once
#include <L3.h>

namespace L3 {

class LabelGlobalizer {
public:
  static std::string generateNewName() { return prefix + std::to_string(count++); }
  static void initialize(Program *P);

private:
  static std::string prefix;
  static int count;
  static bool initialized;
};

void globalizeLabels(Program *P);

} // namespace L3