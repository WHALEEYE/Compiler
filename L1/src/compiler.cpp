#include <algorithm>
#include <assert.h>
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

void print_help(char *progName) {
  std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] SOURCE"
            << std::endl;
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
  int32_t opt;
  while ((opt = getopt(argc, argv, "vg:O:")) != -1) {
    switch (opt) {
    case 'O':
      optLevel = strtoul(optarg, NULL, 0);
      break;

    case 'g':
      enable_code_generator = (strtoul(optarg, NULL, 0) == 0) ? false : true;
      break;

    case 'v':
      verbose = true;
      break;

    default:
      print_help(argv[0]);
      return 1;
    }
  }

  /*
   * Parse the input file.
   */
  auto p = L1::parse_file(argv[optind]);

  /*
   * Code optimizations (optional)
   */

  /*
   * Generate x86_64 assembly.
   */
  if (enable_code_generator) {
    L1::generate_code(p);
  }

  return 0;
}
