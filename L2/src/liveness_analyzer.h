#pragma once

#include <L2.h>

#include <map>
#include <unordered_set>

namespace L2 {

class LivenessResult;

class LivenessSets {
public:
  const std::unordered_set<Symbol *> &getGEN() const;
  const std::unordered_set<Symbol *> &getKILL() const;
  const std::unordered_set<Symbol *> &getIN() const;
  const std::unordered_set<Symbol *> &getOUT() const;

private:
  std::unordered_set<Symbol *> GEN, KILL, IN, OUT;

  friend const LivenessResult &analyzeLiveness(Program &P);
  friend bool analyzeInBB(BasicBlock *BB, std::map<Instruction *, LivenessSets> &result,
                          bool visited);
  friend void calculateGenKill(Function *F, std::map<Instruction *, LivenessSets> &result);
};

class LivenessResult {
public:
  LivenessResult() = default;
  void printResult(Function *F) const;
  const std::map<Instruction *, LivenessSets> &getFunctionResult(Function *F) const;

private:
  std::map<Function *, std::map<Instruction *, LivenessSets>> result;

  LivenessResult &operator=(const LivenessResult &) = delete;
  LivenessResult(const LivenessResult &) = delete;

  friend const LivenessResult &analyzeLiveness(Program &P);
  friend bool analyzeInBB(BasicBlock *BB, std::map<Instruction *, LivenessSets> &result,
                          bool visited);
  friend void calculateGenKill(Function *F, std::map<Instruction *, LivenessSets> &result);
};

const LivenessResult &analyzeLiveness(Program &P);

} // namespace L2