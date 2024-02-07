#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdint.h>
#include <unistd.h>

using namespace std;

#include <L3.h>
#include <code_generator.h>
#include <helper.h>
#include <label_globalizer.h>
#include <parser.h>

void printHelp(char *progName) {
  cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l] [-i] [-d] SOURCE" << endl;
  return;
}

int main(int argc, char **argv) {
  auto enableCodeGenerator = true;
  int32_t optLevel = 3;

  /*
   * Check the compiler arguments.
   */
  auto verbose = false;
  if (argc < 2) {
    printHelp(argv[0]);
    return 1;
  }
  int32_t opt;
  int64_t functionNumber = -1;
  while ((opt = getopt(argc, argv, "vg:O:d")) != -1) {
    switch (opt) {
    case 'O':
      optLevel = strtoul(optarg, NULL, 0);
      break;

    case 'g':
      enableCodeGenerator = (strtoul(optarg, NULL, 0) == 0) ? false : true;
      break;

    case 'v':
      verbose = true;
      break;

    case 'd':
      debugEnabled = true;
      break;

    default:
      printHelp(argv[0]);
      return 1;
    }
  }

  /*
   * Parse the input file.
   */
  L3::Program *P = L3::parseFile(argv[optind]);

  if (verbose) {
    cout << "Program before globalizing labels:" << endl;
    cout << P->toStr();
    L3::globalizeLabels(P);
    cout << "Program after globalizing labels:" << endl;
    cout << P->toStr();
  }

  /*
   * Generate the target code.
   */
  if (enableCodeGenerator) {
  }

  return 0;
}
