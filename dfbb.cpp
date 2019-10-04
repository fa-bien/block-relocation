#include <algorithm>

#include "dfbb.h"

extern unique_ptr<BRPPolicy> ubSolver;
extern bool verbose;

DFBB::DFBB(unsigned int UB, unsigned int timeLimit) {
    UB_ = UB;
    timeLimit_ = timeLimit;
}
    
shared_ptr<BRPState> DFBB::solve(const BRPState &initialState) const {
    clock_t startTicks = 0;
    if (timeLimit_ > 0) {
        startTicks = clock();
    }
    // 
    BRPState currentState(initialState);
    clock_t before = clock();
    shared_ptr<BRPState> bestFound = ubSolver->solve(initialState);
    cout << "Calculated UB in " << ((double)clock() - before) / CLOCKS_PER_SEC
         << " seconds" << endl;
    unsigned int bestObj = min(UB_, bestFound->nRelocations());
    //
    cout << "Starting builtin depth-first branch-and-bound with LB = "
         << currentState.LB()
         << " and UB = " << bestObj << endl;
    bool finished = solveSub(currentState, -1, bestFound, bestObj, startTicks);
    if (! finished) {
        cout << "DFBB: Time limit reached!" << endl;
    }
    return bestFound;
}

// returns false if time limit reached, true otherwise
// side effect: bestFound and bestObj are updated if a new better solution
// is found
bool DFBB::solveSub(BRPState &currentState,
                    unsigned int lastRelocatedTo,
                    shared_ptr<BRPState> &bestFound,
                    unsigned int &bestObj,
                    const clock_t &startTicks) const {
    // cout << "\t from subroutine, timeLimit_ = " << timeLimit_
    //      << endl;
    // step 0: do we still have time?
    if ( timeLimit_ > 0 &&
         ((double) clock() - startTicks) / CLOCKS_PER_SEC > timeLimit_){
        return false;
    }
    
    // step 1: perform all possible retrievals
    int nRetrievals = 0;
    while (currentState.retrieveNext()) {
        nRetrievals += 1;
        lastRelocatedTo = -1;
    }
    // are we done?
    // if yes: did we find a new best solution?
    if (currentState.empty()) {
        if (currentState.nRelocations() < bestObj) {
            bestObj = currentState.nRelocations();
            bestFound = make_shared<BRPState>(currentState);
        }
    } else if (currentState.nRelocations() + currentState.LB() >= bestObj) {
        // if ( currentState.nRelocations() + currentState.LB2() < bestObj) {
        //     cout << currentState << endl;
        //     cout << "LB1 = " << currentState.LB1() << endl;
        //     cout << "LB2 = " << currentState.LB2() << endl;
        //     cout << "LB3 = " << currentState.LB3() << endl;
        // }        
        ;
    } else { // step 2: branch and evaluate subtrees

        // // special case: do we have depth+LB = UB - 1?
        // // if yes: compute UB at this node
        // if ( currentState.nRelocations() + currentState.LB() == bestObj - 1 ) {
        //     shared_ptr<BRPState> ns = ubSolver->solve(currentState);
        //     if ( ns->nRelocations() < bestObj ) {
        //         cout << "tralala" << endl;
        //         bestObj = ns->nRelocations();
        //         bestFound = ns;
        //     }
        // }
        
        // each possible relocation is a branch
        // a branch is a <LB, from, to> tuple
        vector<tuple<unsigned int, unsigned int, int> > branches;
        int currentLB = currentState.LB1();
        for (unsigned int sFrom=0; sFrom < currentState.W(); sFrom++) {
            // only relocate from stacks with at least one item and which
            // are not the last stack we relocated to
            if ( sFrom != lastRelocatedTo &&
                 currentState.height(sFrom) > 0 ) {
                // item being relocated
                unsigned int item = currentState.top(sFrom);
                // look-ahead part 1: difference on LB induced by relocating
                // item from sFrom
                int fromDiff = 0;
                if (currentState.low(sFrom) < item) {
                    fromDiff = -1;
                }
                bool relocatedToEmpty = false;
                // now try every destination stack
                for (unsigned int sTo=0; sTo < currentState.W(); sTo++) {
                    if (sTo != sFrom &&  // do not relocate to same stack
                        currentState.height(sTo) < currentState.H()) {
                        // only relocate to an empty stack once to
                        // break symmetry
                        if (relocatedToEmpty &&
                            currentState.height(sTo) == 0) {
                            continue;
                        }
                        // look-ahead part 2: difference on LB induced by
                        // relocating item to sTo
                        int toDiff = 0;
                        if (currentState.low(sTo) < item) {
                            toDiff = 1;
                        }
                        int newBound = currentState.nRelocations() + 1 +
                            currentLB + fromDiff + toDiff;
                        // would that move be promising?
                        if (newBound < bestObj) {
                            branches.push_back(make_tuple(newBound,
                                                          sFrom, sTo));
                        }
                    }
                }
            }
        }
        // now that all branches are computed, sort them from most to least
        // promising and try them all
        sort(branches.begin(), branches.end());
        bool keepGoing = true;
        for (auto branch: branches) {
            // 2.1: branch
            currentState.relocate(get<1>(branch), get<2>(branch));
            // 2.2: evaluate subtree
            keepGoing = solveSub(currentState,
                                 get<2>(branch),
                                 bestFound,
                                 bestObj,
                                 startTicks);
            // 2.3: cancel branching decision
            currentState.undoLastMove();
            // 2.4: it time limit has been reached, stop everything
            if (! keepGoing) {
                return false;
            }
        }
    }
    // step 3: cancel retrievals
    while (nRetrievals > 0) {
        currentState.undoLastMove();
        nRetrievals -= 1;
    }
    return true;
}

shared_ptr<BRPState> DFBBLoop::solve(const BRPState &initialState) const {
    clock_t startTicks = 0;
    if (timeLimit_ > 0) {
        startTicks = clock();
    }

    if ( verbose ) {
        cout << "Starting DFBB-Loop with time limit = " << timeLimit_ << endl;
    }
    
    //
    BRPState currentState(initialState);
    shared_ptr<BRPState> bestFound = ubSolver->solve(initialState);
    unsigned int UB = min(UB_, bestFound->nRelocations());
    //
    unsigned int LB = currentState.LB3();
    if ( verbose ) {
        cout << "Starting builtin DFBB-loop with LB = "
             << currentState.LB()
             << " and UB = " << UB << endl;
    }
    cout << "Starting builtin depth-first branch-and-bound with LB = "
         << currentState.LB()
         << " and UB = " << UB << endl;
    if (LB == UB) {
        if ( verbose ) {
            cout << "Done at root node!!" << endl;
        }
        return bestFound;
    } else {
        unsigned int UBcur = LB;
        while (UBcur < UB) {
            unsigned int bestObj = UBcur + 1;
            BRPState tmpState(initialState);
            if ( verbose ) {
                cout << "*** Trying with UB = " << bestObj << endl;
                cout << "\t calling subroutine, timeLimit_ = " << timeLimit_
                     << endl;
            }
            
            bool finished = solveSub(tmpState, -1, bestFound, bestObj,
                                     startTicks);
            if (! finished) {
                cerr << "DFBB-Loop: Time limit reached!" << endl;
                return bestFound;
            }
            if ( verbose ) { 
                cout << "\tcurrent best found: " << bestFound->nRelocations()
                     << endl;
            }
            UB = min(UB, bestFound->nRelocations());
            UBcur += 1;
        }
        return bestFound;
    }
}


DFBBLoop::DFBBLoop(unsigned int UB, unsigned int timeLimit) {
    UB_ = UB;
    timeLimit_ = timeLimit;
}
