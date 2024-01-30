#pragma once

#include <L2.h>

#include <map>
#include <unordered_set>
#include <vector>

namespace L2 {

class LivenessResult;

class LivenessSets {
public:
  const std::unordered_set<const Symbol *> &getGEN() const;
  const std::unordered_set<const Symbol *> &getKILL() const;
  const std::unordered_set<const Symbol *> &getIN() const;
  const std::unordered_set<const Symbol *> &getOUT() const;

private:
  std::unordered_set<const Symbol *> GEN, KILL, IN, OUT;

  friend const LivenessResult &analyzeLiveness(const Function *F);
  friend bool analyzeInBB(const BasicBlock *BB, LivenessResult &functionResult, bool visited);
  friend void calculateGenKill(const Function *F, LivenessResult &functionResult);
};

class LivenessResult {
public:
  LivenessResult() = default;
  void dump() const;
  const LivenessSets &getLivenessSets(const Instruction *I) const;

private:
  std::map<const Instruction *, LivenessSets> result;
  std::vector<const Instruction *> instBuffer;

  LivenessResult &operator=(const LivenessResult &) = delete;
  LivenessResult(const LivenessResult &) = delete;

  friend const LivenessResult &analyzeLiveness(const Function *F);
  friend bool analyzeInBB(const BasicBlock *BB, LivenessResult &functionResult, bool visited);
  friend void calculateGenKill(const Function *F, LivenessResult &functionResult);
};

const LivenessResult &analyzeLiveness(const Function *F);

} // namespace L2