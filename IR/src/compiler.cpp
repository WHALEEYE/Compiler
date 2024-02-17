#include <cstdlib>
#include <iostream>
#include <unistd.h>

using namespace std;

#include <code_generator.h>
#include <helper.h>
#include <label_globalizer.h>
#include <parser.h>

void printHelp(char *progName) {
  cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l] [-i] [-d] SOURCE" << endl;
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
      optLevel = strtoul(optarg, nullptr, 0);
      break;

    case 'g':
      enableCodeGenerator = strtoul(optarg, nullptr, 0) != 0;
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
  }
  L3::globalizeLabels(P);
  if (verbose) {
    cout << "Program after globalizing labels:" << endl;
    cout << P->toStr();
  }

  /*
   * Generate the target code.
   */
  if (enableCodeGenerator) {
    unordered_map<const L3::Function *, const L3::TilingResult &> programTilingResult;
    for (auto F : P->getFunctions()) {
      auto &liveness = L3::analyzeLiveness(F);
      auto &trees = L3::constructTrees(F, liveness);
      auto &tilingResult = L3::tileFunction(trees);
      programTilingResult.insert({F, tilingResult});
    }
    L3::generate_code(programTilingResult, P);
  }

  return 0;
}
