#include <fstream>
#include <iostream>
#include <string>

#include <code_generator.h>

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
  // TODO

  /*
   * Close the output file.
   */
  outputFile.close();

  return;
}
} // namespace L1
