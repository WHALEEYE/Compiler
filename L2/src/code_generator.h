#pragma once

#include <unordered_map>

#include <L2.h>
#include <graph_colorer.h>

using namespace std;

namespace L2 {

void generate_code(Program *P, unordered_map<const Function *, const ColorResult *> &results);

}
