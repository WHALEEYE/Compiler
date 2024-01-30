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
  void setSpilledVar(const Variable *spilledVar);
  std::string getSpillPrefix() const;
  void setSpillPrefix(std::string spillPrefix);

private:
  const Variable *spilledVar;
  std::string spillPrefix;
  bool spilled;
};

struct VarSpillInfo {
  const MemoryLocation *memLoc = nullptr;
  const Variable *newVar = nullptr;
};

class SpillInfo {
public:
  SpillInfo(std::string spillPrefix);
  std::string consumeName();
  bool isSpilled(const Variable *var) const;
  VarSpillInfo *getVarSpillInfo(const Variable *var);
  void dump() const;

private:
  std::unordered_map<const Variable *, VarSpillInfo> varSpillInfos;
  int64_t spillCount, nextPostfix;
  std::string spillPrefix;
};

void spillProgram(Program *P, const LivenessResult &livenessResult);

void spillFunction(Function *F, SpillInfo &spillInfo, const LivenessResult &livenessResult,
                   const std::unordered_set<const Variable *> &varsToBeSpilled);
} // namespace L2