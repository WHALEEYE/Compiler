#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <set>
#include <stdint.h>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

#include <code_generator.h>
#include <parser.h>

using namespace std;

void print_help(char *progName) {
  std::cerr << "Usage: " << progName << " SOURCE" << std::endl;
  return;
}

int main(int argc, char **argv) {
  auto enable_code_generator = false;
  int32_t optLevel = 0;
  bool verbose;

  /*
   * Check the compiler arguments.
   */
  if (argc < 2) {
    print_help(argv[0]);
    return 1;
  }

  /*
   * Parse the input file.
   */
  auto p = L1::parse_file(argv[optind]);

  /*
   * Interpret the L1 program.
   */
  // TODO

  return 0;
}
