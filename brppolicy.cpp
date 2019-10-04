#include <iostream>

#include "brppolicy.h"

using namespace std;

// return number of moves necessary to empty the bay
shared_ptr<BRPState> BRPPolicy::solve(const BRPState &s1) const {
    // cout << "Solving with " << name() << endl;
    BRPState state(s1);
    while (true) {
        // retrieve the next item if possible
        if (state.retrieveNext()) {
            // was that the last one?
            if (state.empty()) {
                return make_shared<BRPState>(state);
            }
            // if not possible, try voluntary moves
        } else if (! voluntaryMoves(state) ) {
            // if no voluntary move is possible, perform a forced move
            forcedMove(state);
        }
    }
    return make_shared<BRPState>(state);
}

// relocate container n as in LA heuristics
void BRPPolicy::laRelocate(BRPState &state, unsigned int n) const {
    int bestS = bestDestForLaRelocate(state, n);
    state.relocate(state.stackForItem(n), bestS);
}

// relocate container n as in LA heuristics
unsigned int BRPPolicy::bestDestForLaRelocate(BRPState &state,
                                              unsigned int n) const {
    int bestS = -1;
    for (unsigned int s = 0; s < state.W(); s++) {
        // cases that we can skip
        if ( s == state.stackForItem(n) || state.height(s) == state.H() ) {
            continue;
        } else if ( bestS == -1 ) { // first valid stack we try
            bestS = s;
        } else if ( state.low(s) > n ) {
            if ( state.low(bestS) < n ||
                 ( state.low(bestS) > n && state.low(s) < state.low(bestS) ) ) {
                // this is always an improvement
                bestS = s;
            }
        } else if ( state.low(s) < n &&
                    state.low(bestS) < n &&
                    state.low(s) > state.low(bestS) ) {
            // we generate a conflict but it will happen later than with the
            // previous bestS
            bestS = s;
        }
    }
    return bestS;
}

// perform a forced move
// meant to be overloaded in derived classes
void BRPPolicy::forcedMove(BRPState &state) const {
    unsigned int n = state.next();
    // cout << "\t forced move: relocating "
    //      << state.top(state.stackForItem(n)) << endl;
    laRelocate(state, state.top(state.stackForItem(n)));
}

// retrieve items that can be retrieved
void BRPPolicy::autoRetrieve(BRPState &state) const {
    while (state.retrieveNext());
}
