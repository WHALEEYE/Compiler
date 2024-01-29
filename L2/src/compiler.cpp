#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdint.h>
#include <unistd.h>

#include <L2.h>
#include <code_generator.h>
#include <interference_analyzer.h>
#include <liveness_analyzer.h>
#include <parser.h>
#include <spiller.h>

void printHelp(char *progName) {
  std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l] [-i] [-d] SOURCE"
            << std::endl;
  return;
}

int main(int argc, char **argv) {
  auto enableCodeGenerator = true;
  auto spillOnly = false;
  auto interferenceOnly = false;
  auto livenessOnly = false;
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
  while ((opt = getopt(argc, argv, "vg:O:slid")) != -1) {
    switch (opt) {

    case 'l':
      livenessOnly = true;
      break;

    case 'i':
      interferenceOnly = true;
      break;

    case 's':
      spillOnly = true;
      break;

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
  L2::Program *P;

  if (spillOnly) {

    /*
     * Parse an L2 function and the spill arguments.
     */
    P = L2::parseSpillFile(argv[optind]);

  } else if (livenessOnly) {

    /*
     * Parse an L2 function.
     */
    P = L2::parseFunctionFile(argv[optind]);

  } else if (interferenceOnly) {

    /*
     * Parse an L2 function.
     */
    P = L2::parseFunctionFile(argv[optind]);

  } else {

    /*
     * Parse the L2 program.
     */
    P = L2::parseFile(argv[optind]);
  }

  if (verbose) {
    std::cout << "(" << P->getEntryPointLabel() << std::endl;
    for (auto F : P->getFunctions()) {
      std::cout << "  (" << F->getName() << "\n    " << F->getParamNum() << "\n";

      for (auto BB : F->getBasicBlocks())
        for (auto I : BB->getInstructions())
          std::cout << "    " << I->toStr() << std::endl;

      std::cout << "  )" << std::endl;
    }
    std::cout << ")" << std::endl;
  }

  /*
   * Special cases.
   */
  if (spillOnly) {

    /*
     * Spill.
     */
    auto &livenessResult = L2::analyzeLiveness(P);
    L2::spillProgram(P, livenessResult);

    for (auto F : P->getFunctions()) {
      auto spilledF = (L2::FunctionToSpill *)F;
      std::cout << "(" << spilledF->getName() << "\n\t" << spilledF->getParamNum() << " "
                << (spilledF->getSpilled() ? 1 : 0) << "\n";

      for (auto BB : F->getBasicBlocks())
        for (auto I : BB->getInstructions())
          std::cout << "\t" << I->toStr() << std::endl;

      std::cout << ")" << std::endl;
    }

    return 0;
  }

  /*
   * Liveness test.
   */
  if (livenessOnly) {
    auto &livenessResult = L2::analyzeLiveness(P);
    livenessResult.getFunctionResult(P->getCurrFunction()).dump();
    return 0;
  }

  /*
   * Interference graph test.
   */
  if (interferenceOnly) {
    auto &livenessResult = L2::analyzeLiveness(P);
    auto &interferenceResult = L2::analyzeInterference(P, livenessResult);
    interferenceResult.getFunctionGraph(P->getCurrFunction()).dump();
    return 0;
  }

  /*
   * Generate the target code.
   */
  if (enableCodeGenerator) {
    // TODO
  }

  return 0;
}
