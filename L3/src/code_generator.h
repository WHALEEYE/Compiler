#pragma once
#include <L3.h>
#include <string>

using namespace std;

namespace L3 {

vector<string> generateAssign(string lhs, string rhs);
vector<string> generateCompare(string rst, string lhs, string op, string rhs);
vector<string> generateLoad(string val, string addr);
vector<string> generateStore(string addr, string val);
vector<string> generateArithmetic(string rst, string lhs, string op, string rhs);
vector<string> generateBranch(string label);
vector<string> generateCondBranch(string cond, string label);
vector<string> generateReturn();
vector<string> generateReturnVal(string val);
vector<string> generateCall(string callee, vector<string> args);
vector<string> generateLabel(string label);

void generate_code(Program *P);

} // namespace L3
