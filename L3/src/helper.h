#pragma once
#include <L3.h>
#include <string>

extern bool debugEnabled;

void debug(std::string message);

template <typename T> bool isa(void *node) { return dynamic_cast<const T *>(node) != nullptr; }