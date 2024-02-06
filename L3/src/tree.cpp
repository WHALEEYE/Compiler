#include <tree.h>

namespace L3 {

const Tile *TreeNode::getMatchedTile() const { return matchedTile; }
void TreeNode::setMatchedTile(const Tile *tile, bool edge) {
  matchedTile = tile;
  status = edge ? Status::PARTIAL_MATCHED : Status::MATCHED;
}
TreeNode::Status TreeNode::getStatus() const { return status; }

} // namespace L3