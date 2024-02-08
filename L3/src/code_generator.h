#pragma once
#include <string>

#include <L3.h>
#include <tile.h>
#include <vector>

using namespace std;

namespace L3 {

vector<string> generateAssign(string lhs, string rhs);
vector<string> generateCompare(string rst, string lhs, string op, string rhs);
vector<string> generateLoad(string val, string addr);
vector<string> generateStore(string addr, string val);
vector<string> generateArithmetic(const OperandNode *rst, const OperandNode *lhs,
                                  const ArithmeticNode *op, const OperandNode *rhs);
vector<string> generateBranch(string label);
vector<string> generateCondBranch(string cond, string label);
vector<string> generateReturn();
vector<string> generateReturnVal(string val);
vector<string> generateCall(string callee, vector<string> args);
vector<string> generateCallAssign(string rst, string callee, vector<string> args);
vector<string> generateLabel(string label);

void generate_code(const TilingResult &result, Program *P);

} // namespace L3
