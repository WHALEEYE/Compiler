#pragma once

#include <L2.h>

namespace L2 {

class ProgramToSpill : public Program {
public:
  ProgramToSpill();
  bool isSpilled() const;
  void setSpilled(bool spilled);
  std::string getSpilledVar() const;
  void setSpilledVar(std::string spilledVar);
  std::string getSpillPrefix() const;
  void setSpillPrefix(std::string spillPrefix);

private:
  std::string spilledVar;
  std::string spillPrefix;
  bool spilled;
};

void SpillProgram(ProgramToSpill *P);
} // namespace L2