#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include <code_generator.h>
#include <helper.h>
#include <loop_analyzer.h>
#include <parser.h>

using namespace std;

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
  LB::Program *P = LB::parseFile(argv[optind]);
  unordered_map<const LB::Function *, const LB::LoopInfo &> loopInfos;

  if (verbose)
    cout << "read:\n" << P->toStr();

  for (auto F : P->getFunctions())
    loopInfos.insert({F, LB::analyzeLoops(F)});

  if (verbose)
    cout << "analyzed loops:\n" << P->toStr();

  /*
   * Generate the target code.
   */
  if (enableCodeGenerator) {
    generate_code(P, loopInfos);
  }

  return 0;
}
