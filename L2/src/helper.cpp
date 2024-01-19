#include <helper.h>

bool debugEnabled = false;

template <typename T> DebugStream &DebugStream::operator<<(const T &value) {
  if (debugEnabled)
    std::cerr << "\033[33m" << value << "\033[0m";
  return *this;
}

typedef std::ostream &(*StreamManipulator)(std::ostream &);
DebugStream &DebugStream::operator<<(StreamManipulator manip) {
  if (debugEnabled)
    manip(std::cerr);
  return *this;
}

DebugStream debug;
