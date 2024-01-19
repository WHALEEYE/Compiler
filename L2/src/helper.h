#pragma once
#include <iostream>

extern bool debugEnabled;

class DebugStream {
public:
  template <typename T> DebugStream &operator<<(const T &value);
  typedef std::ostream &(*StreamManipulator)(std::ostream &);
  DebugStream &operator<<(StreamManipulator manip);
};
extern DebugStream debug;