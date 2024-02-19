#pragma once
#include <string>
#include <vector>

#include <L3.h>
#include <tile.h>

using namespace std;

namespace L3 {

vector<string> generateAssign(const string &lhs, const string &rhs);
vector<string> generateCompare(const string &rst, const string &lhs, const string &op, const string &rhs);
vector<string> generateLoad(const string &val, const string &addr);
vector<string> generateStore(const string &addr, const string &val);
vector<string> generateArithmetic(const OperandNode *rst, const OperandNode *lhs, const ArithmeticNode *op,
                                  const OperandNode *rhs);
vector<string> generateBranch(const string &label);
vector<string> generateCondBranch(const string &cond, const string &label);
vector<string> generateReturn();
vector<string> generateReturnVal(const string &val);
vector<string> generateCall(const string &callee, vector<string> args);
vector<string> generateCallAssign(const string &rst, const string &callee, vector<string> args);
vector<string> generateLabel(const string &label);

void generate_code(const unordered_map<const Function *, const TilingResult &> &result, Program *P);

} // namespace L3
