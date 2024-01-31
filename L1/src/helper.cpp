#include <helper.h>

#include <iostream>

bool debugEnabled = false;

void debug(std::string message) {
  if (debugEnabled)
    std::cerr << "\033[33m[DEBUG] " << message << "\033[0m" << std::endl;
}