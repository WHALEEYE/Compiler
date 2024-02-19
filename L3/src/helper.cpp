#include <iostream>

#include <helper.h>
using namespace std;

bool debugEnabled = false;

void debug(const std::string& message) {
  if (debugEnabled)
    cerr << "\033[33m[DEBUG] " << message << "\033[0m" << endl;
}