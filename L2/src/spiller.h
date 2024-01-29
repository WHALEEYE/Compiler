#pragma once

#include <L2.h>
#include <liveness_analyzer.h>

namespace L2 {

class FunctionToSpill : public Function {
public:
  FunctionToSpill(std::string name);
  bool getSpilled() const;
  void setSpilled(bool spilled);
  const Variable *getSpilledVar() const;
  void setSpilledVar(Variable *spilledVar);
  std::string getSpillPrefix() const;
  void setSpillPrefix(std::string spillPrefix);

private:
  Variable *spilledVar;
  std::string spillPrefix;
  bool spilled;
};

void spillProgram(Program *P, const LivenessResult &livenessResult);
} // namespace L2