#pragma once

#include <map>
#include <unordered_set>
#include <vector>

#include <L3.h>

namespace L3 {

class LivenessResult;

class LivenessSets {
public:
  const std::unordered_set<const Variable *> &getGEN() const;
  const std::unordered_set<const Variable *> &getKILL() const;
  const std::unordered_set<const Variable *> &getIN() const;
  const std::unordered_set<const Variable *> &getOUT() const;

private:
  std::unordered_set<const Variable *> GEN, KILL, IN, OUT;

  friend const LivenessResult &analyzeLiveness(const Function *F);
  friend bool analyzeInBB(const BasicBlock *BB, LivenessResult &functionResult, bool visited);
  friend void calculateGenKill(const Function *F, LivenessResult &functionResult);
};

class LivenessResult {
public:
  LivenessResult() = default;
  void dump() const;
  const LivenessSets &getLivenessSets(const Instruction *I) const;

  LivenessResult &operator=(const LivenessResult &) = delete;
  LivenessResult(const LivenessResult &) = delete;

private:
  std::map<const Instruction *, LivenessSets> result;
  std::vector<const Instruction *> instBuffer;

  friend const LivenessResult &analyzeLiveness(const Function *F);
  friend bool analyzeInBB(const BasicBlock *BB, LivenessResult &functionResult, bool visited);
  friend void calculateGenKill(const Function *F, LivenessResult &functionResult);
};

const LivenessResult &analyzeLiveness(const Function *F);

} // namespace L2