#include "L1.h"
#include <code_generator.h>
#include <fstream>

using namespace std;

namespace L1 {
void generate_code(Program p) {

  /*
   * Open the output file.
   */
  std::ofstream outputFile;
  outputFile.open("prog.S");

  /*
   * Generate target code
   */
  outputFile << ".text" << endl
             << "  .globl go" << endl
             << "go:" << endl
             << "  pushq %rbx" << endl
             << "  pushq %rbp" << endl
             << "  pushq %r12" << endl
             << "  pushq %r13" << endl
             << "  pushq %r14" << endl
             << "  pushq %r15" << endl
             << "  call _" << p.entryPointLabel.substr(1) << endl
             << "  popq %r15" << endl
             << "  popq %r14" << endl
             << "  popq %r13" << endl
             << "  popq %r12" << endl
             << "  popq %rbp" << endl
             << "  popq %rbx" << endl
             << "  retq" << endl;

  for (auto f : p.functions) {
    outputFile << "_" + f->name.substr(1) << ":" << endl;
    if (f->locals > 0)
      outputFile << "  subq $" << f->locals * 8 << ", %rsp" << endl;

    for (auto i : f->instructions) {
      auto indent = true;
      if (i->getX86Inst() == "")
        continue;
      else if (i->getX86Inst() == "retq") {
        int amount = (f->parameters > 6 ? (f->parameters - 6) * 8 : 0) + f->locals * 8;
        if (amount > 0)
          outputFile << "  addq $" << amount << ", %rsp" << endl;
      } else if (dynamic_cast<LabelInst *>(i))
        indent = false;
      outputFile << (indent ? "  " : "") << i->getX86Inst() << endl;
    }
  }
  /*
   * Close the output file.
   */
  outputFile.close();

  return;
}
} // namespace L1
