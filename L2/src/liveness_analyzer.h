#pragma once

#include <L2.h>

#include <map>
#include <unordered_set>
#include <vector>

namespace L2 {

class LivenessResult;
class FunctionLivenessResult;

class LivenessSets {
public:
  const std::unordered_set<Symbol *> &getGEN() const;
  const std::unordered_set<Symbol *> &getKILL() const;
  const std::unordered_set<Symbol *> &getIN() const;
  const std::unordered_set<Symbol *> &getOUT() const;

private:
  std::unordered_set<Symbol *> GEN, KILL, IN, OUT;

  friend const LivenessResult &analyzeLiveness(Program &P);
  friend bool analyzeInBB(BasicBlock *BB, FunctionLivenessResult &result, bool visited);
  friend void calculateGenKill(Function *F, FunctionLivenessResult &result);
};

class FunctionLivenessResult {
public:
  FunctionLivenessResult() = default;
  void dump() const;

private:
  std::map<Instruction *, LivenessSets> result;
  std::vector<Instruction *> instBuffer;

  FunctionLivenessResult &operator=(const FunctionLivenessResult &) = delete;
  FunctionLivenessResult(const FunctionLivenessResult &) = delete;

  friend const LivenessResult &analyzeLiveness(Program &P);
  friend bool analyzeInBB(BasicBlock *BB, FunctionLivenessResult &functionResult, bool visited);
  friend void calculateGenKill(Function *F, FunctionLivenessResult &result);
};

class LivenessResult {
public:
  LivenessResult() = default;
  const FunctionLivenessResult &getFunctionResult(Function *F) const;

private:
  std::map<Function *, FunctionLivenessResult> functionResults;

  LivenessResult &operator=(const LivenessResult &) = delete;
  LivenessResult(const LivenessResult &) = delete;

  friend const LivenessResult &analyzeLiveness(Program &P);
  friend bool analyzeInBB(BasicBlock *BB, FunctionLivenessResult &functionResult, bool visited);
  friend void calculateGenKill(Function *F, FunctionLivenessResult &result);
};

const LivenessResult &analyzeLiveness(Program &P);

} // namespace L2