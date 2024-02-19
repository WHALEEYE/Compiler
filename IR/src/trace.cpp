#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std;

#include <IR.h>
#include <helper.h>
#include <trace.h>

namespace IR {

struct Edge {
  BasicBlock *from;
  BasicBlock *to;
  int64_t profit;
};

class Edges {
public:
  int64_t getProfit(const BasicBlock *from, const BasicBlock *to) const {
    auto edge = getEdge(from, to);
    if (!edge)
      throw runtime_error("Edge not found");
    return edge->profit;
  }

  void addProfit(BasicBlock *from, BasicBlock *to, int64_t profit) {
    auto edge = getSafeEdge(from, to);
    edge->profit += profit;
  }

  bool profitable(const BasicBlock *from, const BasicBlock *to, const unordered_set<BasicBlock *> &seen) const {
    int64_t profit = getProfit(from, to);
    int64_t maxProfit = 0;

    for (auto pred : to->getPredecessors()) {
      if (seen.find(pred) != seen.end())
        continue;
      maxProfit = max(maxProfit, getProfit(pred, to));
    }

    return profit >= maxProfit;
  }

  void finalize() {
    for (auto &entry : edgeMap)
      for (auto &edge : entry.second)
        edges.push_back(edge.second);

    sort(edges.begin(), edges.end(), [](Edge *a, Edge *b) { return a->profit > b->profit; });
  }

  const vector<Edge *> &getEdges() const { return edges; }

private:
  Edge *getEdge(const BasicBlock *from, const BasicBlock *to) const {
    if (edgeMap.find(from) == edgeMap.end())
      return nullptr;
    auto it = edgeMap.at(from).find(to);
    if (it != edgeMap.at(from).end())
      return it->second;
    return nullptr;
  }

  Edge *getSafeEdge(BasicBlock *from, BasicBlock *to) {
    auto edge = getEdge(from, to);
    if (edge)
      return edge;
    edge = new Edge{from, to, 0};
    edgeMap[from][to] = edge;
    return edge;
  }

  vector<Edge *> edges;
  unordered_map<const BasicBlock *, unordered_map<const BasicBlock *, Edge *>> edgeMap;
};

unordered_set<BasicBlock *> walkBB(BasicBlock *curr, unordered_set<BasicBlock *> &seen, Edges &result) {

  debug("Walking BB " + curr->toStr());
  if (seen.find(curr) != seen.end())
    return {curr};

  unordered_set<BasicBlock *> loopHeads;
  seen.insert(curr);

  int64_t baseProfit = 0;
  if (curr->getSuccessors().size() == 1)
    baseProfit = 1;

  for (auto succ : curr->getSuccessors()) {
    result.addProfit(curr, succ, baseProfit);

    auto subHeads = walkBB(succ, seen, result);
    if (subHeads.empty())
      continue;
    result.addProfit(curr, succ, 1);
    subHeads.erase(curr);
    loopHeads.insert(subHeads.begin(), subHeads.end());
  }
  seen.erase(curr);
  return loopHeads;
}

const Edges &analyzeEdges(BasicBlock *entryBlock) {
  auto &result = *(new Edges());
  unordered_set<BasicBlock *> seen;
  walkBB(entryBlock, seen, result);
  result.finalize();
  return result;
}

BasicBlock *selectNextBB(const Edges &edges, const unordered_set<BasicBlock *> &seen) {
  BasicBlock *next = nullptr;

  auto &candidates = edges.getEdges();
  // first, traverse in order
  for (auto edge : candidates) {
    if (seen.find(edge->to) != seen.end() || seen.find(edge->from) != seen.end())
      continue;
    // if find a edge that both from and to are not seen, select it
    next = edge->from;
    break;
  }

  if (next)
    return next;

  // fallback: traverse in reverse order
  for (auto it = candidates.rbegin(); it != candidates.rend(); ++it) {
    auto edge = *it;
    if (seen.find(edge->to) != seen.end())
      continue;

    next = edge->to;
    break;
  }

  return next;
}

void rearrangeBBs(Function *F) {
  vector<BasicBlock *> newBBs, oldBBs = F->getBasicBlocks();
  unordered_set<BasicBlock *> seen;
  auto &edges = analyzeEdges(oldBBs.front());
  seen.insert(oldBBs.front());

  BasicBlock *curr = oldBBs.front();
  while (newBBs.size() < oldBBs.size()) {
    if (!curr)
      curr = selectNextBB(edges, seen);
    if (!curr)
      throw runtime_error("No more BBs to add");

    newBBs.push_back(curr);
    seen.insert(curr);

    int64_t maxProfit = -1;
    BasicBlock *maxSucc = nullptr;
    for (auto succ : curr->getSuccessors()) {
      if (seen.find(succ) != seen.end())
        continue;

      if (edges.profitable(curr, succ, seen) && edges.getProfit(curr, succ) > maxProfit) {
        maxProfit = edges.getProfit(curr, succ);
        maxSucc = succ;
      }
    }
    curr = maxSucc;
  }

  F->basicBlocks = newBBs;
}

} // namespace IR