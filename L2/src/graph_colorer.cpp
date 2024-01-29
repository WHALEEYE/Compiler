#include <graph_colorer.h>

namespace L2 {

void colorGraph(const InterferenceResult &interferenceResult, const LivenessResult &livenessResult) {
    /*
        1. find a prefix that no variable shares at first, which will be used as the prefix of spilled variables
        2. design an order of colors to be used
        3. enter a loop
            - liveness analysis and interference graph construction
            - subloop1: find a node that has less than K neighbors
                - if no such node, break
                - if such node, push it to the stack
            - subloop2: push other nodes into stack
                - from the node that has the most neighbors
                - if no such node, break
                - if such node, push it to the stack
            - pop a node from the stack
                - assign a color for it if possible
                - if not possible, mark it as no color
            - for all the nodes that are marked as no color, try to spill them (for those who are not spilled already, check by prefix)
                - if all of them are spilled. break
        4. If reached here and still not all nodes are colored, then spill all.
    */
}
}