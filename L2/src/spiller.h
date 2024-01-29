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
  int64_t nextPostfix = 0;
  const MemoryLocation *memLoc = nullptr;
  const Variable *newVar = nullptr;
};

class FunctionSpillInfo {
public:
  bool isSpilled(const Variable *var) const;
  VarSpillInfo *getVarSpillInfo(const Variable *var);

private:
  std::unordered_map<const Variable *, VarSpillInfo> varSpillInfos;
  int64_t spillCount = 0;
};

void spillProgram(Program *P, const LivenessResult &livenessResult);

void spillFunction(Function *F, const FunctionLivenessResult &livenessResult,
                                 const std::unordered_set<const Variable *> &varsToBeSpilled,
                                 std::string spillPrefix, FunctionSpillInfo &functionSpillInfo);
} // namespace L2