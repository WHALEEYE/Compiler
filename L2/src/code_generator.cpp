#include <L2.h>
#include <code_generator.h>
#include <fstream>

using namespace std;

namespace L2 {
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

  for (auto f : p.functions)
    outputFile << "_" + f->name.substr(1) << ":" << endl;
  /*
   * Close the output file.
   */
  outputFile.close();

  return;
}
} // namespace L2
