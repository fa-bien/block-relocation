#include <iostream>
#include <deque>

#include "branchandbound.h"
#include "rakesearch.h"
#include "petering.h"

extern unique_ptr<BRPPolicy> ubSolver;

const int BranchAndBound::breadthFirst = 1;
const int BranchAndBound::depthFirst = 2;

BranchAndBound::BranchAndBound(unsigned int UB,
                               string explorationStrategy,
                               unsigned int timeLimit) {
    UB_ = UB;
    explorationStrategy_ = explorationStrategy;
    timeLimit_ = timeLimit;
}

shared_ptr<BRPState> BranchAndBound::solve(const BRPState &initialState) const {
    clock_t startTicks = 0;
    if (timeLimit_ > 0) {
        startTicks = clock();
    }
    // 
    deque<shared_ptr<BRPState> > Q;
    Q.push_back( make_shared<BRPState>(initialState));
    // unsigned int bestKnown = min(UB_, LA_N(1).solve(initialState));
    // storage of best solution
    shared_ptr<BRPState> bestState = ubSolver->solve(initialState);
    unsigned int bestKnown = min(UB_, bestState->nRelocations());
    //
    cout << "Starting " << explorationStrategy_
         << "-first branch-and-bound with LB = " << Q.back()->LB()
         << " and UB = " << bestKnown << endl;
    int strategy = -1;
    if (explorationStrategy_ == "breadth") {
        strategy = breadthFirst;
    } else if (explorationStrategy_ == "depth") {
        strategy = depthFirst;
    } else {
        cerr << "unknown branch-and-bound strategy: " << explorationStrategy_
             << endl;
        exit(22);
    }
    while (Q.size() > 0) {
        // check for time limit if necessary
        if (timeLimit_ > 0) {
            if ( ((double) clock() - startTicks) / CLOCKS_PER_SEC > timeLimit_){
                cout << "Branch-and-bound: time limit reached!" << endl;
                int lowestLB = 1e9;
                for (auto i : Q) {
                    if (i->LB() + i->nRelocations() < lowestLB) {
                        lowestLB = i->LB() + i->nRelocations();
                    }
                }
                cout << "current LB = " << lowestLB << endl;
                cout << "Remaining nodes to process: " << Q.size() << endl;
                return bestState;
            }
        }
        shared_ptr<BRPState> tmpState;
        if (strategy == breadthFirst) {
            tmpState = Q.front();
            Q.pop_front();
        } else if (strategy == depthFirst) {
            tmpState = Q.back();
            Q.pop_back();
        }
        // can we fathom this node?
        if (tmpState->LB() + tmpState->nRelocations() >= bestKnown) {
            continue;
        }
        // general case: we need to branch
        // Step 1: perform all possible retrievals
        while (tmpState->retrieveNext());
        // are we done?
        if (tmpState->empty()) {
            if (tmpState->nRelocations() < bestKnown) {
                bestKnown = tmpState->nRelocations();
                bestState = tmpState;
            }
        } else { // Step 2: generate successors
            for (unsigned int sFrom=0; sFrom < tmpState->W(); sFrom++) {
                // only relocate from stacks with at least one item and which
                // are not the last stack we relocated to
                if ( sFrom != tmpState->lastRelocatedTo() &&
                     tmpState->height(sFrom) > 0 ) {
                    unsigned int item = tmpState->top(sFrom);
                    int fromDiff = 0;
                    if (tmpState->low(sFrom) < item) {
                        fromDiff = -1;
                    }
                    bool relocatedToEmpty = false;
                    for (unsigned int sTo=0; sTo < tmpState->W(); sTo++) {
                        if (sTo != sFrom &&  // do not relocate to same stack
                            tmpState->height(sTo) < tmpState->H()) {
                            // only relocate to an empty stack once to
                            // break symmetry
                            if (relocatedToEmpty &&
                                tmpState->height(sTo) == 0) {
                                // cout << "skipping relocation to empty stack"
                                //      << endl;
                                continue;
                            }
                            int toDiff = 0;
                            if (tmpState->low(sTo) < item) {
                                toDiff = 1;
                            }
                            int newBound = tmpState->nRelocations() + 1 +
                                tmpState->LB() + fromDiff + toDiff;
                            // would that move be promising?
                            if (newBound < bestKnown) {
                                shared_ptr<BRPState> newState =
                                    make_shared<BRPState>(BRPState(*tmpState));
                                newState->relocate(sFrom, sTo);
                                Q.push_back(newState);
                                // did we just relocate to an empty stack?
                                if (newState->height(sTo) == 1) {
                                    relocatedToEmpty = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    cout << "Branch-and-bound is over" << endl;
    return bestState;
}
