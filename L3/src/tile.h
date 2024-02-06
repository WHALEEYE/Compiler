#pragma once
#include <unordered_map>

#include <L3.h>

namespace L3 {
class TreeNode;

class Tile {
public:
  enum class Type { ARITH, ASSIGN, STORE, LOAD, CALL, RET, RET_VAL, BR, COND_BR, PHI, LABEL, FUNC };

private:
  Type type;
  static const std::unordered_map<Type, const Tile &> enumMap;
};

class ArithTile : public Tile {
public:
  enum class ArithOp { ADD, SUB, MUL, AND, LEFT_SHIFT, RIGHT_SHIFT };
  int match(TreeNode *node);
  static const ArithTile &getInstance() {
    static const ArithTile instance;
    return instance;
  }

private:
  ArithTile() {};
  ArithTile(const ArithTile &) = delete;
  ArithTile &operator=(const ArithTile &) = delete;
};

} // namespace L3