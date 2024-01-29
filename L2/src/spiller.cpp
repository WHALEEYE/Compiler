
#include <L2.h>
#include <spiller.h>

namespace L2 {
ProgramToSpill::ProgramToSpill() : Program() {
  this->entryPointLabel = "@<Spill>";
  spilled = false;
}

bool ProgramToSpill::isSpilled() const { return spilled; }
void ProgramToSpill::setSpilled(bool spilled) { this->spilled = spilled; }
std::string ProgramToSpill::getSpilledVar() const { return spilledVar; }
void ProgramToSpill::setSpilledVar(std::string spilledVar) { this->spilledVar = spilledVar; }
std::string ProgramToSpill::getSpillPrefix() const { return spillPrefix; }
void ProgramToSpill::setSpillPrefix(std::string spillPrefix) { this->spillPrefix = spillPrefix; }

void SpillProgram(ProgramToSpill *P) {
  auto newMemLoc = new MemoryLocation(Register::getRegister(Register::ID::RSP), new Number(0));
  for (auto F : P->getFunctions()) {
    int spillCount = 0;
    for (auto BB : F->getBasicBlocks()) {
      std::vector<Instruction *> spilledInsts;
      for (auto I : BB->getInstructions()) {
        spilledInsts.push_back(I);

        // do actions according to the instruction type
        if (auto callInst = dynamic_cast<CallInst *>(I)) {

          // if callee is not a variable, skip
          if (!dynamic_cast<Variable *>(callInst->getCallee()))
            continue;

          auto var = dynamic_cast<Variable *>(callInst->getCallee());
          if (var->getName() != P->getSpilledVar())
            continue;

          auto newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
          // pop the original call instruction
          spilledInsts.pop_back();
          spilledInsts.push_back(new AssignInst(newVar, newMemLoc));
          spilledInsts.push_back(new CallInst(newVar, callInst->getArgNum()));
        } else if (auto assignInst = dynamic_cast<AssignInst *>(I)) {
          auto rval = assignInst->getRval();
          auto lval = assignInst->getLval();
          auto needStore = false, needLoad = false;
          Variable *newVar = nullptr;

          // check left value
          if (auto lvar = dynamic_cast<Variable *>(lval)) {
            if (lvar->getName() == P->getSpilledVar()) {
              needStore = true;
              if (!newVar)
                newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
              lval = newVar;
            }
          } else if (auto memLoc = dynamic_cast<MemoryLocation *>(lval)) {
            if (auto var = dynamic_cast<Variable *>(memLoc->getBase())) {
              if (var->getName() == P->getSpilledVar()) {
                needLoad = true;
                if (!newVar)
                  newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
                lval = new MemoryLocation(newVar, memLoc->getOffset());
              }
            }
          }

          // check right value
          if (auto rvar = dynamic_cast<Variable *>(rval)) {
            if (rvar->getName() == P->getSpilledVar()) {
              needLoad = true;
              if (!newVar)
                newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
              rval = newVar;
            }
          } else if (auto memLoc = dynamic_cast<MemoryLocation *>(rval)) {
            if (auto var = dynamic_cast<Variable *>(memLoc->getBase())) {
              if (var->getName() == P->getSpilledVar()) {
                needLoad = true;
                if (!newVar)
                  newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
                rval = new MemoryLocation(newVar, memLoc->getOffset());
              }
            }
          }

          if (!newVar)
            continue;

          spilledInsts.pop_back();
          if (needLoad)
            spilledInsts.push_back(new AssignInst(newVar, newMemLoc));
          spilledInsts.push_back(new AssignInst(lval, rval));
          if (needStore)
            spilledInsts.push_back(new AssignInst(newMemLoc, newVar));
        } else if (auto shiftInst = dynamic_cast<ShiftInst *>(I)) {
          auto lval = shiftInst->getLval();
          auto rval = shiftInst->getRval();
          auto needStore = false;
          Variable *newVar = nullptr;

          // check left value
          if (auto lvar = dynamic_cast<Variable *>(lval)) {
            if (lvar->getName() == P->getSpilledVar()) {
              needStore = true;
              if (!newVar)
                newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
              lval = newVar;
            }
          }

          // check right value
          if (auto rvar = dynamic_cast<Variable *>(rval)) {
            if (rvar->getName() == P->getSpilledVar()) {
              if (!newVar)
                newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
              rval = newVar;
            }
          }

          if (!newVar)
            continue;

          spilledInsts.pop_back();
          spilledInsts.push_back(new AssignInst(newVar, newMemLoc));
          spilledInsts.push_back(new AssignInst(lval, rval));
          if (needStore)
            spilledInsts.push_back(new AssignInst(newMemLoc, newVar));
        } else if (auto arithInst = dynamic_cast<ArithInst *>(I)) {
          auto rval = arithInst->getRval();
          auto lval = arithInst->getLval();
          auto needStore = false;
          Variable *newVar = nullptr;

          // check left value
          if (auto lvar = dynamic_cast<Variable *>(lval)) {
            if (lvar->getName() == P->getSpilledVar()) {
              needStore = true;
              if (!newVar)
                newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
              lval = newVar;
            }
          } else if (auto memLoc = dynamic_cast<MemoryLocation *>(lval)) {
            if (auto var = dynamic_cast<Variable *>(memLoc->getBase())) {
              if (var->getName() == P->getSpilledVar()) {
                if (!newVar)
                  newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
                lval = new MemoryLocation(newVar, memLoc->getOffset());
              }
            }
          }

          // check right value
          if (auto rvar = dynamic_cast<Variable *>(rval)) {
            if (rvar->getName() == P->getSpilledVar()) {
              if (!newVar)
                newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
              rval = newVar;
            }
          } else if (auto memLoc = dynamic_cast<MemoryLocation *>(rval)) {
            if (auto var = dynamic_cast<Variable *>(memLoc->getBase())) {
              if (var->getName() == P->getSpilledVar()) {
                if (!newVar)
                  newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
                rval = new MemoryLocation(newVar, memLoc->getOffset());
              }
            }
          }

          if (!newVar)
            continue;

          spilledInsts.pop_back();
          spilledInsts.push_back(new AssignInst(newVar, newMemLoc));
          spilledInsts.push_back(new ArithInst(arithInst->getOp(), lval, rval));
          if (needStore)
            spilledInsts.push_back(new AssignInst(newMemLoc, newVar));
        } else if (auto selfModInst = dynamic_cast<SelfModInst *>(I)) {
          if (!dynamic_cast<Variable *>(selfModInst->getLval()))
            continue;

          auto var = dynamic_cast<Variable *>(selfModInst->getLval());
          if (var->getName() != P->getSpilledVar())
            continue;

          auto newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
          spilledInsts.pop_back();
          spilledInsts.push_back(new AssignInst(newVar, newMemLoc));
          spilledInsts.push_back(new SelfModInst(selfModInst->getOp(), newVar));
          spilledInsts.push_back(new AssignInst(newMemLoc, newVar));
        } else if (auto compAssignInst = dynamic_cast<CompareAssignInst *>(I)) {
          auto lval = compAssignInst->getLval();
          auto cmpLval = compAssignInst->getCmpLval();
          auto cmpRval = compAssignInst->getCmpRval();
          auto needStore = false, needLoad = false;
          Variable *newVar = nullptr;

          // check left value
          if (auto lvar = dynamic_cast<Variable *>(lval)) {
            if (lvar->getName() == P->getSpilledVar()) {
              needStore = true;
              if (!newVar)
                newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
              lval = newVar;
            }
          }

          // check compare left value
          if (auto cmpLvar = dynamic_cast<Variable *>(cmpLval)) {
            if (cmpLvar->getName() == P->getSpilledVar()) {
              needLoad = true;
              if (!newVar)
                newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
              cmpLval = newVar;
            }
          }

          // check compare right value
          if (auto cmpRvar = dynamic_cast<Variable *>(cmpRval)) {
            if (cmpRvar->getName() == P->getSpilledVar()) {
              needLoad = true;
              if (!newVar)
                newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
              cmpRval = newVar;
            }
          }

          if (!newVar)
            continue;

          spilledInsts.pop_back();
          if (needLoad)
            spilledInsts.push_back(new AssignInst(newVar, newMemLoc));
          spilledInsts.push_back(
              new CompareAssignInst(lval, compAssignInst->getOp(), cmpLval, cmpRval));
          if (needStore)
            spilledInsts.push_back(new AssignInst(newMemLoc, newVar));
        } else if (auto condJumpInst = dynamic_cast<CondJumpInst *>(I)) {
          auto cmpLval = condJumpInst->getLval();
          auto cmpRval = condJumpInst->getRval();
          Variable *newVar = nullptr;

          // check compare left value
          if (auto cmpLvar = dynamic_cast<Variable *>(cmpLval)) {
            if (cmpLvar->getName() == P->getSpilledVar()) {
              if (!newVar)
                newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
              cmpLval = newVar;
            }
          }

          // check compare right value
          if (auto cmpRvar = dynamic_cast<Variable *>(cmpRval)) {
            if (cmpRvar->getName() == P->getSpilledVar()) {
              if (!newVar)
                newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
              cmpRval = newVar;
            }
          }

          if (!newVar)
            continue;

          spilledInsts.pop_back();
          spilledInsts.push_back(new AssignInst(newVar, newMemLoc));
          spilledInsts.push_back(
              new CondJumpInst(condJumpInst->getOp(), cmpLval, cmpRval, condJumpInst->getLabel()));
        } else if (auto setInst = dynamic_cast<SetInst *>(I)) {
          auto lval = setInst->getLval();
          auto base = setInst->getBase();
          auto offset = setInst->getOffset();
          auto needStore = false, needLoad = false;
          Variable *newVar = nullptr;

          // check left value
          if (auto lvar = dynamic_cast<Variable *>(lval)) {
            if (lvar->getName() == P->getSpilledVar()) {
              needStore = true;
              if (!newVar)
                newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
              lval = newVar;
            }
          }

          // check base
          if (auto baseVar = dynamic_cast<Variable *>(base)) {
            if (baseVar->getName() == P->getSpilledVar()) {
              needLoad = true;
              if (!newVar)
                newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
              base = newVar;
            }
          }

          // check offset
          if (auto offsetVar = dynamic_cast<Variable *>(offset)) {
            if (offsetVar->getName() == P->getSpilledVar()) {
              needLoad = true;
              if (!newVar)
                newVar = new Variable(P->getSpillPrefix() + std::to_string(spillCount++));
              offset = newVar;
            }
          }

          if (!newVar)
            continue;

          spilledInsts.pop_back();
          if (needLoad)
            spilledInsts.push_back(new AssignInst(newVar, newMemLoc));
          spilledInsts.push_back(new SetInst(lval, base, offset, setInst->getScalar()));
          if (needStore)
            spilledInsts.push_back(new AssignInst(newMemLoc, newVar));
        }
      }
      BB->instructions = spilledInsts;
      P->setSpilled(spillCount > 0);
    }
  }
}

} // namespace L2