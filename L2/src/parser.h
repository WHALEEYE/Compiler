#pragma once

#include <L2.h>

namespace L2 {
Program parseFile(char *fileName);

Program parseSpillFile(char *fileName);

Program parseFunctionFile(char *fileName);
} // namespace L2
